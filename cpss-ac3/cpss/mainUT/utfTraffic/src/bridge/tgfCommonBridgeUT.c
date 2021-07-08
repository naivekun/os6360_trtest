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
* @file tgfCommonBridgeUT.c
*
* @brief Enhanced UTs for CPSS Bridge
*
* @version   86
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <bridge/prvTgfBasicDynamicLearning.h>
#include <bridge/prvTgfTransmitAllKindsOfTraffic.h>
#include <bridge/prvTgfVlanIngressFiltering.h>
#include <bridge/prvTgfVlanEgressFiltering.h>
#include <bridge/prvTgfVlanManipulation.h>
#include <bridge/prvTgfVlanFloodVidx.h>
#include <bridge/prvTgfFdbAging.h>
#include <bridge/prvTgfBrgGen.h>
#include <bridge/prvTgfBrgSrcId.h>
#include <bridge/prvTgfVlanTag1RemIfZero.h>
#include <bridge/prvTgfVlanKeepVlan1.h>
#include <bridge/prvTgfBrgIeeeReservedMcastLearning.h>
#include <bridge/prvTgfBrgRateLimitMcastReg.h>
#include <bridge/prvTgfFdbAuNaMessageExtFormat.h>
#include <bridge/prvTgfBrgVlanPortAccFrameType.h>
#include <bridge/prvTgfBrgProtocolBasedVlanLlc.h>
#include <bridge/prvTgfBrgInvalidVlanFiltering.h>
#include <bridge/prvTgfBrgEgrTagModifNonDsaTaggedEgrPort.h>
#include <bridge/prvTgfBrgNestedCustomerToCore.h>
#include <bridge/prvTgfFdbChangeInterfaceType.h>
#include <bridge/prvTgfFdbChangeDaCommand.h>
#include <bridge/prvTgfFdbChangeSaCommand.h>
#include <bridge/prvTgfFdbTriggeredAgingEntries.h>
#include <bridge/prvTgfFdbTriggeredAgingVlans.h>
#include <bridge/prvTgfFdbDelMacEntriesForVlanPort.h>
#include <bridge/prvTgfMcGroupRxMirror.h>
#include <bridge/prvTgfFdbSourceIdModeDefault.h>
#include <bridge/prvTgfFdbSourceIdModeChange.h>
#include <bridge/prvTgfBrgCustomerBpdus.h>
#include <bridge/prvTgfBrgProviderBpdus.h>
#include <bridge/prvTgfFdbLearningAdressesSvlMode.h>
#include <bridge/prvTgfMcBridgingIpV4.h>
#include <bridge/prvTgfMcBridgingIpV6.h>
#include <bridge/prvTgfBrgDroppingIpV6MacMulticastFiltering.h>
#include <bridge/prvTgfBrgDroppingNonIpV4MacMulticastFiltering.h>
#include <bridge/prvTgfFdbPerInlifPortUnregIpv6McFiltering.h>
#include <bridge/prvTgfFdbPerInlifPortUnregIpv4BcFiltering.h>
#include <bridge/prvTgfFdbSecurityBreach.h>
#include <bridge/prvTgfFdbPerInlifPortUnknownUcFiltering.h>
#include <bridge/prvTgfFdbPerEgressPortUnknownUcFiltering.h>
#include <bridge/prvTgfFdbPerEgressPortUnregIpv4McFiltering.h>
#include <bridge/prvTgfFdbPerEgressPortUnregIpv6BcFiltering.h>
#include <bridge/prvTgfFdbLocalUcSwitching.h>
#include <bridge/prvTgfFdbLocalMcSwitching.h>
#include <bridge/prvTgfBrgNestedCoreToCustomer.h>
#include <bridge/prvTgfBrgVlanIndexing.h>
#include <bridge/prvTgfBrgVlanForceNewDsa.h>

#include <bridge/prvTgfBrgUntaggedMruCheck.h>
#include <bridge/prvTgfBrgIngressFilteringDisableStp.h>
#include <bridge/prvTgfBrgStpEgressFilteringChangeStates.h>
#include <bridge/prvTgfBrgIngressCountersSecFilterDisc.h>
#include <bridge/prvTgfBrgHostCounters.h>
#include <bridge/prvTgfBrgMatrixCounters.h>
#include <bridge/prvTgfBrgIngressCountersSpanningTreeStateDrop.h>
#include <bridge/prvTgfBrgEgressCountersUnknownPacketsTcDp.h>
#include <bridge/prvTgfBrgEgressCountersCtrlToAnlyzrPortVlan.h>
#include <bridge/prvTgfBrgBasicJumboFrame.h>
#include <bridge/prvTgfFdbSecureAutoLearnMovedSa.h>
#include <bridge/prvTgfBrgMtuCheck.h>
#include <bridge/prvTgfVlanEDsaTagHandling.h>
#include <bridge/prvTgfBrgArpTrapping.h>
#include <bridge/prvTgfBrgVlanTagPop.h>
#include <bridge/prvTgfBrgGenPropL2ControlProtCmd.h>
#include <bridge/prvTgfBrgGenPropL2ControlProtCmdAf.h>
#include <bridge/prvTgfBrgGenIcmpV6.h>
#include <bridge/prvTgfBrgGenVid1LearnFDBCheck.h>
#include <bridge/prvTgfFdbBasicIpv6UcRouting.h>
#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>
#include <bridge/prvTgfFdbIpv4UcRoutingLookupMask.h>
#include <bridge/prvTgfFdbIpv6UcRoutingLookupMask.h>
#include <bridge/prvTgfFdbIpv4UcRoutingDeleteEnable.h>
#include <bridge/prvTgfFdbIpv6UcRoutingDeleteEnable.h>
#include <bridge/prvTgfFdbIpv4UcRoutingRefreshEnable.h>
#include <bridge/prvTgfFdbIpv6UcRoutingRefreshEnable.h>
#include <bridge/prvTgfFdbIpv4UcRoutingTransplantEnable.h>
#include <bridge/prvTgfFdbIpv6UcRoutingTransplantEnable.h>
#include <bridge/prvTgfFdbIpv4UcRoutingAgingEnable.h>
#include <bridge/prvTgfFdbIpv6UcRoutingAgingEnable.h>
#include <bridge/prvTgfBrgVlanInvalidate.h>
#include <bridge/prvTgfFdbIpv4UcPointerRoute.h>
#include <ip/prvTgfBasicIpv4UcEcmpRouting.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <bridge/prvTgfBrgVplsBasicTest.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfCscdGen.h>
#include <bridge/prvTgfBrgVlanFws.h>
#include <common/tgfMirror.h>
#include <bridge/prvTgfFdbIpv4v6Uc2VrfRouting.h>
#include <bridge/prvTgfFdbIpv4UcRoutingMtuCheckNonDf.h>
#include <bridge/prvTgfFdbAuNaMessage.h>
#include <bridge/prvTgfBrgAsicSimulationPerformance.h>


/* AUTODOC:
   run test <tr101Test> and check when
   traffic egress cascade port --> DSA/eDSA
*/
void tr101Test_egressCascadePort(IN UT_TEST_FUNC_TYPE tr101Test)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /*only devices that support set of cascade port for Tx direction that is not coupled with Rx direction */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E         |    UTF_CH1_DIAMOND_E |
                                                      UTF_CH2_E         |    UTF_CH3_E         |
                                                      UTF_XCAT_E | UTF_XCAT3_E        |    UTF_XCAT2_E);

    if( GT_TRUE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return ;
    }


    if(prvTgfPortsArray[0] > 63 || prvTgfPortsArray[1] > 63 ||
       prvTgfPortsArray[2] > 63 || prvTgfPortsArray[3] > 63)
    {
        /* the tests are 'hard coded' with low port numbers */
        prvUtfSkipTestsSet();
        return ;
    }

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    prvTgfVlanManipulationTestAsDsa(TGF_DSA_2_WORD_TYPE_E);
    tr101Test();

    if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        /* the device not supports the eDSA */
        return;
    }

    prvTgfVlanManipulationTestAsDsa(TGF_DSA_4_WORD_TYPE_E);
    tr101Test();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic dynamic MAC learning:
    send traffic with different MAC SA on each port;
    verify that got traffic on each port;
    verify dynamically created FDB entries;
    send traffic with learned MACs;
    verify traffic forwarded according to dynamically created FDB entries.
*/
/*
    Create VLAN[portIdx]: 2[0, 1, 2, 3];
    Send traffic with different MAC SA on each port and verify that got traffic
    on each port;
    Read dynamically created FDB entries and verify MACs, VLANs, ports;
    Send traffic with learned MACs and verify that traffic forwarded correctly
    according to dynamically created FDB entries.
*/
UTF_TEST_CASE_MAC(tgfBasicDynamicLearning)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);
    /* Set configuration */
    prvTgfBrgBdlConfigurationSet();

    /* Generate traffic */
    prvTgfBrgBdlTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgBdlConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of tgfBasicDynamicLearning , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'forward' ('extended')
            1. the test run in 2 modes : 'bypass bridge' and  NOT 'bypass bridge'
            2. the SA learning from the 'DSA info' will be on 'remote device'
            3. the DSA info hold target dev,port to local port on the DUT.
               so in 'bypass bridge' the egress is to single port
               when 'NOT bypass bridge' and DA is unknown --> flood to all ports
               when 'NOT bypass bridge' and DA is known --> egress in single port
            4. there is use of :
               a. device map table in mode (trg dev,trg port),
               b. set port as cascade for ingress (but not for egress)
               c. 'bypass bridge' and NOT 'bypass bridge' (for DSA tagged packets)

*/
UTF_TEST_CASE_MAC(tgfBasicDynamicLearning_fromCascadePortDsaTagForward)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);
    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    {   /* skip multi device boards for cascading related tests */
        GT_U32      boardIdx, boardRevId, reloadEeprom;

        cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

        /* skip multidevice boards */
        if((boardIdx == 31) /* Lion2 + BC2 */)
        {
            SKIP_TEST_MAC;
        }
    }

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   0, 1, 2, 3,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    if( GT_TRUE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
    {
        /* the test need to set the device map table to mode of {trgDev,trgPort}
           and use this mode for 'known UC' to remote device ,
           but the 'multi-core FDB lookup' also use the device map table ,
           but the AppDemo did not set the table for this mode of {trgDev,trgPort}

           --> so sending unknown will not behave correctly in mode of {trgDev,trgPort}

           NOTE: when 'no bypass bridge' , there are no sending of unknown !
           */
    }
    else
    {
        /* state that we run in DSA tag mode - no bypass bridge */
        prvTgfBrgBdlConfigurationDsaTagEnableSet(GT_TRUE/*use DSA*/,GT_FALSE/*no bypass bridge*/,TGF_DSA_2_WORD_TYPE_E);

        /* run the test - tgfBasicDynamicLearning */
        UTF_TEST_CALL_MAC(tgfBasicDynamicLearning);
    }

    /* state that we run in DSA tag mode - bypass bridge*/
    prvTgfBrgBdlConfigurationDsaTagEnableSet(GT_TRUE/*use DSA*/,GT_TRUE/*bypass bridge*/,TGF_DSA_2_WORD_TYPE_E);

    /* run the test - tgfBasicDynamicLearning */
    UTF_TEST_CALL_MAC(tgfBasicDynamicLearning);

    /****************************/
    /* run with 4 words DSA tag */
    /****************************/

    /* state that we run in DSA tag mode - bypass bridge*/
    if(GT_TRUE == prvTgfBrgBdlConfigurationDsaTagEnableSet(GT_TRUE/*use DSA*/,GT_TRUE/*bypass bridge*/,TGF_DSA_4_WORD_TYPE_E))
    {
        /* run the test - tgfBasicDynamicLearning */
        UTF_TEST_CALL_MAC(tgfBasicDynamicLearning);
    }

    /* state that we run in DSA tag mode - no bypass bridge*/
    if(GT_TRUE == prvTgfBrgBdlConfigurationDsaTagEnableSet(GT_TRUE/*use DSA*/,GT_FALSE/*no bypass bridge*/,TGF_DSA_4_WORD_TYPE_E))
    {
        /* run the test - tgfBasicDynamicLearning */
        UTF_TEST_CALL_MAC(tgfBasicDynamicLearning);
    }


}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test transmitting all kinds of traffic from all ports:
    Unknown unicast;
    Known unicast forwarded to port;
    Broadcast;
    Known unicast forwarded to VIDX.
*/
/*
    Create VLAN[portIdx]: 3[0, 1, 2, 3];
    Set default port vlanId 3 on portIdx 0;
    Add portIdx 0 as MC member for unknown VIDX 0;
    Create FDB entry on portIdx 2, vlan 3;
    Create MC Group with VIDX 1 and portIdx members [0, 1];
    Create FDB entry on VIDX 2, vlan 3;
    Send unknown UC traffic from portIdx 0 and verify traffic on all ports;
    Send known UC traffic from portIdx 1 and verify traffic is
    forwarded according to FDB entry to portIdx 2;
    Send BC traffic from portIdx 2 and verify traffic on all ports;
    Send registered MC traffic from portIdx 3 and verify is routed according to
    MC group portIdx [2, 3].
*/
UTF_TEST_CASE_MAC(tgfTransmitAllKindsOfTraffic)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Check CRC mode hash calculation
    4. Restore configuration

*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgTransmitAllConfigurationSet();

    /* Generate traffic */
    prvTgfBrgTransmitAllTrafficGenerate();

    /* Check CRC mode hash calculation */
    prvTgfBrgCrcHashCalcModeTest();

    /* Restore configuration */
    prvTgfBrgTransmitAllConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN ingress filtering tagged packets:
    create 2 different VLANs;
    enable\disable ingress filtering on send port;
    send tagged traffic with different tags;
    verify to receive traffic on correct VLANs.
*/
/*
    Create 2 VLANs[portIdx]: 2[0, 1], 3[2, 3];
    Set Port not Member in VLAN Drop counter mode;
    Enable ingress filtering on portIdx 0;
    Send tagged traffic with vlanId 2 on portIdx 0 and vefify to get traffic
    on portIdx [0, 1] and verify drop counters;
    Send tagged traffic with vlanId 3 on portIdx 0 and vefify to get no traffic
    and verify drop counters;
    Disable ingress filtering on portIdx 0;
    Send tagged traffic with vlanId 3 on portIdx 0 and vefify to get traffic
    on portIdx [2, 3] and verify drop counters.
*/
UTF_TEST_CASE_MAC(tgfBrgVlanIngressFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgVlanIngrFltConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanIngrFltTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanIngrFltConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN egress filtering tagged packets:
    create 2 different VLANs and add FDB entry;
    configure Rx analyzer port and set send port to be mirrored;
    enable\disable egress filtering on send port;
    send tagged traffic with different tags;
    verify to receive traffic on analyzer port.
*/
/*
    Create 2 VLANs[portIdx]: 2[0, 1], 3[2, 3];
    Create 2 FDB entries on portIdx 3, vlans [2, 3];
    Set portIdx 3 as Rx analyzer port and set portIdx 0 to be mirrored;
    Enable egress filtering on portIdx 0;
    Send invalid unknown UC traffic on portIdx 0 and vefify to get traffic
    on portIdx [3];
    Disable egress filtering on portIdx 0;
    Send valid unknown UC traffic on portIdx 0 and vefify to get traffic
    on portIdx [3].
*/
UTF_TEST_CASE_MAC(tgfBrgVlanEgressFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgVlanEgrFltConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanEgrFltTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanEgrFltConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfBrgVlanL2VpnIngressFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA3_E);

    /* Set configuration */
    prvTgfBrgVlanL2VpnIngressFilteringConfigSet();

    /* Generate traffic */
    prvTgfBrgGenVlanL2VpnIngressFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenVlanL2VpnIngressFilteringConfigurationRestore();
}

static GT_BOOL isWithErrorInjection = GT_FALSE;
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - UNTAGGED COMMAND, incoming packet is double tagged:
    configure 2 different VLANs with tag and untag cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet is untagged.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "UNTAGGED";
    Create VLANs[portIdx]: 10[0, 1, 2, 3] with tagging command "TAG0";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 and
    TAG1: ethertype = 0x88a8, vlan = 10 on portIdx 0 and vefify that the output
    packet is untagged.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationUntaggedCmdDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationSet(isWithErrorInjection);

    /* Generate traffic */
    prvTgfBrgVlanManipulationUntaggedCmdDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationUntaggedCmdDoubleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: run scenario of test tgfVlanManipulationUntaggedCmdDoubleTag.
        but before configuration of vlan entry inject error to ALL DFX RAM
        memories that relate to 'vlan table'
        then for every error from HW by interrupt 'fix' from shadow and then retry
        packet again ... till ALL HW errors are gone.(because injected errors to
        multiple HW 'vlan' related tables)
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationUntaggedCmdDoubleTag_withErrorInjection)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
            UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E |
            UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
            UTF_LION_E | UTF_LION2_E | UTF_XCAT3_E |
            UTF_FALCON_E);

    /* GM does not support data integrity */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(prvTgfDevNum))
    {
        SKIP_TEST_MAC;
    }

    isWithErrorInjection = GT_TRUE;
    UTF_TEST_CALL_MAC(tgfVlanManipulationUntaggedCmdDoubleTag);
    isWithErrorInjection = GT_FALSE;

    /* check that the orig test pass */
    UTF_TEST_CALL_MAC(tgfVlanManipulationUntaggedCmdDoubleTag);

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - UNTAGGED COMMAND, incoming packet is single tagged:
    configure VLAN with untag cmd;
    configure ingress\egress TPID config;
    send single tagged traffic with different tags;
    verify output packet is untagged.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "UNTAGGED";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to Ethertype2 = 0x8100, Ethertype3 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype2, TAG3 - is Ethertype1;
    Send tagged traffic with TAG0:  ethertype = 0x8100, vlan = 5 on portIdx 0
    and vefify that the output packet is untagged.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationUntaggedCmdSingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationUntaggedCmdSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationUntaggedCmdSingleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - UNTAGGED COMMAND, incoming packet is untagged:
    configure VLAN with untag cmd;
    configure ingress\egress TPID config;
    send untagged traffic with different tags;
    set PVID for untagged traffic;
    verify output packet is untagged.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "UNTAGGED";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set PVID to 5 - for untagged packet;
    Send untagged traffic on portIdx 0 and vefify that the output packet is untagged.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationUntaggedCmdUntag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationUntaggedCmdUntagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationUntaggedCmdUntagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - TAG0 COMMAND, incoming packet is double tagged:
    configure VLAN with TAG0 cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet has correct TAG0.
*/
/*
    Create VLANs[portIdx]: 25[0, 1, 2, 3] with tagging command "TAG0";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to: Ethertype4 = 0x8100, Ethertype5 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype4, TAG1 - is Ethertype5;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1.
    Set VLAN RANGE to FFF (by default FFE);
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 25 and
    TAG1: ethertype = 0x88a8, vlan = 4095 on portIdx 0 and vefify that the
    output packet has TAG0 with ethertype = 0x9100, vlan = 25.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationTag0CmdDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationTag0CmdDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationTag0CmdDoubleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - TAG0 COMMAND, incoming packet is single tagged:
    configure VLAN with TAG0 cmd;
    configure ingress\egress TPID config;
    send single tagged traffic;
    verify output packet has correct TAG0.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "TAG0";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0
    and vefify that the output packet has TAG0 with ethertype = 0x9100, vlan = 5.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationTag0CmdSingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationTag0CmdSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationTag0CmdSingleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - TAG0 COMMAND, incoming packet is untagged:
    configure VLAN with TAG0 cmd;
    configure ingress\egress TPID config;
    set PVID for untagged traffic;
    send untagged traffic;
    verify output packet has correct TAG0.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "TAG0";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set PVID to 5 - for untagged packet;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send untagged traffic on portIdx 0 and vefify that the output packet has
    TAG0 with ethertype = 0x9100, vlan = 5.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationTag0CmdUntag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationTag0CmdUntagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationTag0CmdUntagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationTag0CmdUntagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - TAG1 COMMAND, incoming packet is double tagged:
    configure VLAN with TAG1 cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet has correct TAG1.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "TAG1";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to: Ethertype6 = 0x8100, Ethertype7 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype6, TAG1 - is Ethertype7;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 and
    TAG1: ethertype = 0x88a8, vlan = 10 on portIdx 0 and vefify that the
    output packet has TAG1 with ethertype = 0x98a8, vlan = 10.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationTag1CmdDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationTag1CmdDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationTag1CmdDoubleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - TAG1 COMMAND, incoming packet is single tagged:
    configure VLAN with TAG1 cmd;
    configure ingress\egress TPID config;
    send single tagged traffic;
    verify output packet has correct TAG1.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "TAG1";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 on
    portIdx 0 and vefify that the output packet has TAG1 with
    ethertype = 0x98a8, vlan = 0.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationTag1CmdSingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationTag1CmdSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationTag1CmdSingleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - TAG1 COMMAND, incoming packet is untagged:
    configure VLAN with TAG1 cmd;
    configure ingress\egress TPID config;
    set PVID for untagged traffic;
    send untagged traffic;
    verify output packet has correct TAG1.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "TAG1";
    Initialize ingress/egress TPID tables to 0;
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set PVID to 5 - for untagged packet;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send untagged traffic portIdx 0 and vefify that the output packet has TAG1
    with ethertype = 0x98a8, vlan = 0.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationTag1CmdUntag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationTag1CmdUntagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationTag1CmdUntagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationTag1CmdUntagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Outer Tag0 Inner Tag1 command, incoming packet is double tagged:
    configure VLAN with Outer TAG0 Inner TAG1  cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet has 2 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Outer Tag0, Inner Tag1";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG1: ethertype = 0x88a8, vlan = 10 and
    TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0 and vefify that the
    output packet has two tags in the following order: TAG0 with
    ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8, vlan = 10.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdDoubleTagConfigurationRestore();
}

/* AUTODOC:
   run test tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag and check when
   traffic egress cascade port --> DSA/eDSA
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_egressCascadePort)
{
    tr101Test_egressCascadePort(UTF_TEST_NAME_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag));
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Outer Tag0 Inner Tag1 command, incoming packet is single tagged:
    configure VLAN with Outer TAG0 Inner TAG1  cmd;
    configure ingress\egress TPID config;
    send single tagged traffic;
    verify output packet has 2 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Outer Tag0, Inner Tag1";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0
    and vefify that the output packet has two tags in the following order: TAG0 with
    ethertype = 0x9100, vlan = 5 and TAG1 with ethertype = 0x98a8, vlan = 0.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdSingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdSingleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Outer Tag0 Inner Tag1 command, incoming packet is untagged:
    configure VLAN with Outer TAG0 Inner TAG1  cmd;
    configure ingress\egress TPID config;
    set PVID for untagged traffic;
    send untagged traffic;
    verify output packet has 2 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Outer Tag0, Inner Tag1";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set PVID to 5 - for untagged packet;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send untagged traffic on portIdx 0 and vefify that the output packet has
    two tags in the following order: TAG0 with ethertype = 0x9100, vlan = 5 and
    TAG1 with ethertype = 0x98a8, vlan = 0.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdUntag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationOuterTag0InnerTag1CmdUntagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Outer Tag1 Inner Tag0 command, incoming packet is double tagged:
    configure VLAN with Outer TAG1 Inner TAG0  cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet has 2 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Outer Tag1, Inner Tag0";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG1: ethertype = 0x88a8, vlan = 10 and
    TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0 and vefify that the
    output packet has two tags in the following order: TAG1 with
    ethertype = 0x98a8, vlan = 10 and TAG0 with ethertype = 0x9100, vlan = 5.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdDoubleTagConfigurationRestore();
}

/* AUTODOC:
   run test tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag and check when
   traffic egress cascade port --> DSA/eDSA
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_egressCascadePort)
{
    tr101Test_egressCascadePort(UTF_TEST_NAME_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag));
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Outer Tag1 Inner Tag0 command, incoming packet is single tagged:
    configure VLAN with Outer TAG1 Inner TAG0  cmd;
    configure ingress\egress TPID config;
    send single tagged traffic;
    verify output packet has 2 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Outer Tag1, Inner Tag0";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0 and
    vefify that the output packet has two tags in the following order: TAG1 with
    ethertype = 0x98a8, vlan = 0 and TAG0 with ethertype = 0x9100, vlan = 5.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdSingleTagConfigurationRestore();
}

/* AUTODOC:
   run test tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag and check when
   traffic egress cascade port --> DSA/eDSA
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_egressCascadePort)
{
    tr101Test_egressCascadePort(UTF_TEST_NAME_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag));
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Outer Tag1 Inner Tag0 command, incoming packet is untagged:
    configure VLAN with Outer TAG1 Inner TAG0  cmd;
    configure ingress\egress TPID config;
    set PVID for untagged traffic;
    send untagged traffic;
    verify output packet has 2 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Outer Tag1, Inner Tag0";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send untagged traffic on portIdx 0 and vefify that the output packet has two
    tags in the following order: TAG1 with ethertype = 0x98a8, vlan = 0 and
    TAG0 with ethertype = 0x9100, vlan = 5.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdUntag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationOuterTag1InnerTag0CmdUntagConfigurationRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Push TAG0 command, incoming packet is double tagged:
    configure VLAN with Push TAG0 cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet has 3 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Push Tag0";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 and
    TAG1: ethertype = 0x88a8, vlan = 10 on portIdx 0 and vefify that the
    output packet has tree tags in the following order:
    TAG0 with ethertype = 0x9100, vlan = 5 and
    TAG0 with ethertype = 0x8100, vlan = 5,
    TAG1 with ethertype = 0x88a8, vlan = 10.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationPushTag0CmdDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationPushTag0CmdDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationPushTag0CmdDoubleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Push TAG0 command, incoming packet is single tagged:
    configure VLAN with Push TAG0 cmd;
    configure ingress\egress TPID config;
    send single tagged traffic;
    verify output packet has 2 correct tags with correct order.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Push Tag0";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0
    and vefify that the output packet has two tags in the following order:
    TAG0 with ethertype = 0x9100, vlan = 5 and TAG0 with ethertype = 0x8100, vlan = 5.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationPushTag0CmdSingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationPushTag0CmdSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationPushTag0CmdSingleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Push TAG0 command, incoming packet is untagged:
    configure VLAN with Push TAG0 cmd;
    configure ingress\egress TPID config;
    set PVID for untagged traffic;
    send untagged traffic;
    verify output packet has correct TAG0.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Push Tag0";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set PVID to 5 - for untagged packet;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send untagged traffic on portIdx 0 and vefify that the output packet has one
    tag: TAG0 with ethertype = 0x9100, vlan = 5.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationPushTag0CmdUntag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationPushTag0CmdUntagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationPushTag0CmdUntagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Pop Outer Tag command, incoming packet is double tagged:
    configure 2 different VLANs with Pop Outer TAG cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet has correct TAG1.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Pop Outer Tag";
    Create VLANs[portIdx]: 10[0, 1, 2, 3] with untagged command;
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 and
    TAG1: ethertype = 0x88a8, vlan = 10 on portIdx 0 and vefify that the
    output packet has one tag: TAG1 with ethertype = 0x98a8, vlan = 10.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationPopOuterTagCmdDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationPopOuterTagCmdDoubleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Pop Outer Tag command, incoming packet is single tagged:
    configure VLAN with Pop Outer TAG cmd;
    configure ingress\egress TPID config;
    send single tagged traffic;
    verify output packet is untagged.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Pop Outer Tag";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8;
    Set ingress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0
    and vefify that the output packet is untagged.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationPopOuterTagCmdSingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationPopOuterTagCmdSingleTagConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Ingress VLAN assignment (1):
    configure 3 different VLANs with TAG0 cmd;
    configure ingress\egress TPID config;
    send 3 tagged packets with different tags;
    verify output packet has correct TAG0.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "TAG0";
    Create VLANs[portIdx]: 25[0, 1, 2, 3] with tagging command "TAG0";
    Create VLANs[portIdx]: 4000[0, 1, 2, 3] with tagging command "TAG0";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8,
    Ethertype2 = 0x5000, Ethertype3 = 0xA0A0, Ethertype4 = 0x5050,
    Ethertype5 = 0x2525, Ethertype6 = 0x5555, Ethertype7 = 0xAAAA;
    Set ingress TPID select to:
    TAG0 - Ethertype0, Ethertype3, Ethertype5, Ethertype6, Ethertype7,
    TAG1 - is Ethertype1, Ethertype2, Ethertype4;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 on portIdx 0
    and vefify that the output packet has TAG0: ethertype =0x9100, vlan = 5;
    Send tagged traffic with TAG0: ethertype = 0xA0A0, vlan = 25 on portIdx 0
    and vefify that the output packet has TAG0: ethertype =0x9100, vlan = 25;
    Send tagged traffic with TAG0: ethertype = 0xAAAA, vlan = 4000 on portIdx 0
    and vefify that the output packet has TAG0: ethertype =0x9100, vlan = 4000.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationIngressVlanAssignment1)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationIngressVlanAssignment1TrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationIngressVlanAssignment1ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Ingress VLAN assignment (2):
    configure VLAN with TAG1 cmd;
    configure ingress\egress TPID config;
    send 2 double tagged packets with different tags;
    verify output packet has correct TAG1.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "TAG1";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8,
    Ethertype2 = 0x5000, Ethertype3 = 0xA0A0, Ethertype4 = 0x5050,
    Ethertype5 = 0x2525, Ethertype6 = 0x5555, Ethertype7 = 0xAAAA;
    Set ingress TPID select to:
    TAG0 - Ethertype0, Ethertype3, Ethertype5, Ethertype6, Ethertype7,
    TAG1 - is Ethertype1, Ethertype2, Ethertype4;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 and
    TAG1: ethertype = 0x88a8, vlan = 25 on portIdx 0 and vefify that the output
    packet has TAG1: ethertype =0x98a8, vlan = 25;
    Send double tagged traffic with TAG0: ethertype = 0xA0A0, vlan = 25 and
    TAG1: ethertype = 0x5050, vlan = 100 on portIdx 0 and vefify that the output
    packet has TAG1:  ethertype =0x98a8, vlan = 100.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationIngressVlanAssignment2)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationIngressVlanAssignment2TrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationIngressVlanAssignment2ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - Change Egress TPID:
    configure VLAN with Outer TAG0 Inner TAG1 cmd;
    configure ingress\egress TPID config;
    send double tagged traffic with different tags;
    verify output packet has correct TAG0 and TAG1;
    update egress TPID table;
    send double tagged traffic with different tags;
    verify output packet has correct TAG0 and TAG1.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tagging command "Outer Tag0, Inner Tag1";
    Set ingress TPID table to: Ethertype0 = 0x8100, Ethertype1 = 0x88a8,
    Ethertype2 = 0x5000, Ethertype3 = 0xA0A0, Ethertype4 = 0x5050,
    Ethertype5 = 0x2525, Ethertype6 = 0x5555, Ethertype7 = 0xAAAA;
    Set ingress TPID select to:
    TAG0 - Ethertype0, Ethertype3, Ethertype5, Ethertype6, Ethertype7,
    TAG1 - is Ethertype1, Ethertype2, Ethertype4;
    Set egress TPID table to: Ethertype0 = 0x9100, Ethertype1 = 0x98a8;
    Set egress TPID select to: TAG0 is Ethertype0, TAG1 - is Ethertype1;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 and
    TAG1: ethertype = 0x88a8, vlan = 100 on portIdx 0 and vefify that the output
    packet has TAG0 with ethertype =0x9100, vlan = 5 and
    TAG1 with ethertype =0x98a8, vlan = 100;
    Set egress TPID table to: Ethertype0 = 0x2222 and Ethertype1 = 0x3333;
    Send double tagged traffic with TAG0: ethertype = 0x8100, vlan = 5 and
    TAG1: ethertype = 0x88a8, vlan = 100 on portIdx 0 and vefify that the output
    packet has TAG0 with ethertype =0x2222, vlan = 5 and
    TAG1 with ethertype =0x3333, vlan = 100.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationChangeEgressTpid)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    /* Set configuration */
    prvTgfBrgVlanManipulationChangeEgressTpidConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanManipulationChangeEgressTpidTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanManipulationChangeEgressTpidConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN TAG MANIPULATION - on Ethernet-Over-Mpls:
   check that the tr101 manipulations are done on the passenger when packet is
   Ethernet-over-X , and the packet did tunnel termination.

   for Bobcat2; Caelum; Bobcat3 device it should check that the TPID profile taken from the ePort
   table and not from the default ePort table.
*/
UTF_TEST_CASE_MAC(tgfVlanManipulationEthernetOverMpls)
{
    GT_U32 testId;/* test ID */
    GT_U32  ii;
    GT_U32  portIndexArr[2] = {5 , 2};
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    if(GT_FALSE == prvTgfBrgVlanTr101Check())
    {
        prvUtfSkipTestsSet();
        return ;
    }

    if(GT_FALSE == prvTgfBrgVlanManipulationEthernetOverMpls_isValid(
        PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST_NO_TUNNEL_E,0))
    {
        prvUtfSkipTestsSet();
        return ;
    }

    if(GT_TRUE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
    {
        /* do not run the test in this mode since the TPID bmp management is
           restricted and the test is not prepared for that */
        prvUtfSkipTestsSet();
        return ;
    }

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    for(ii = 0 ; ii < 2; ii++)
    {
        utfGeneralStateMessageSave(0,"global iteration[%d]",ii);

        for(testId = 0 ;
            testId < PRV_TGF_BRG_VLAN_ETHERNET_OVER_MPLS_CONFIG_TEST____LAST____E;
            testId++)
        {
            if(GT_FALSE == prvTgfBrgVlanManipulationEthernetOverMpls_isValid(
                testId,portIndexArr[ii]))
            {
                continue;
            }

            /* Set configuration */
            prvTgfBrgVlanManipulationEthernetOverMplsConfigurationSet(testId,portIndexArr[ii]);

            /* Generate traffic */
            prvTgfBrgVlanManipulationEthernetOverMplsTrafficGenerate(testId,portIndexArr[ii]);

            /*return;-- AmitK HW debug only -- do not restore the configuration -- stop after first test */

            /* Restore configuration */
            prvTgfBrgVlanManipulationEthernetOverMplsConfigurationRestore(testId,portIndexArr[ii]);

        }

        if(GT_TRUE == prvUtfIsGmCompilation())
        {
            break;
        }
    }
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Bridge MC Port Groups Basic - Unregistred MC:
    configure VLAN entry with unregistered MC mode;
    add MC group to this entry;
    send MC and BC traffic to verify flood vidx mode.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3];
    Configure floodVidxMode = UNREG MC on default VLAN entry (VID = 1);
    Configure floodVidx = 10 on default VLAN entry (VID = 1);
    Build MC Group vidx = 10 with members portIdx [0, 1];
    Send MC traffic on portIdx 0 and vefify MC traffic on portIdx 1;
    Send BC traffic on portIdx 0 and vefify BC traffic on portIdx [1, 2, 3].
*/
UTF_TEST_CASE_MAC(tgfVlanFloodVidx)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgVlanFloodVidxConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanFloodVidxTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanFloodVidxConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Bridge MC Port Groups Basic - All Flooded Traffic:
    configure VLAN entry with ALL flooded traffic mode;
    add MC group to this entry;
    send MC and BC traffic to verify flood vidx mode.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2, 3];
    Configure floodVidxMode = ALL FLOODED TRAFFIC on default VLAN entry (VID = 1);
    Configure floodVidx = 4094 on default VLAN entry (VID = 1);
    Build MC Group vidx = 4094 with members portIdx [0, 1];
    Send MC traffic on portIdx 0 and vefify traffic on portIdx 1;
    Send BC traffic on portIdx 0 and vefify traffic on portIdx 1.
*/
UTF_TEST_CASE_MAC(tgfVlanFloodVidxAllFlooded)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgVlanFloodVidxAllFloodedConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanFloodVidxAllFloodedTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanFloodVidxAllFloodedConfigurationRestore();

}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test FDB Age Bit Da Refresh:
    configure VLAN and FDB entries;
    enable automatic aging with removal;
    send packet to check auto-aging;
    enable DA-based aging;
    send packet avoid MAC entry aging and check DA refresh.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Create FDB entry on portIdx 3, vlan 2;
    Set trigger mode to automatic mode;
    Enable automatic aging with removal;
    Send packet on portIdx 0 to check auto-aging - flooding unknown UC is expected;
    Enable DA-based aging;
    Create new FDB non-static entry on portIdx 3, vlan 2;
    Send packet on portIdx 0 avoid MAC entry aging and check DA refresh.
*/
UTF_TEST_CASE_MAC(tgfFdbAgeBitDaRefresh)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
   /*
        1. the multi-port groups device not support auto aging with removal ,
           so this test is not applicable for it
        2. the multi-port group device can't refresh with DA hit all the port
           groups , so this test in not applicable for it.

    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* the test is long due to sleep of at least 45 seconds */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* GM not supports aging daemon */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* Set configuration */
    prvTgfFdbAgeBitDaRefreshConfigurationSet();

    /* Generate traffic */
    prvTgfFdbAgeBitDaRefreshTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbAgeBitDaRefreshConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test for multi-port groups device - check that AA is not received from 'not associated' port group:
    configure VLAN and FDB entries;
    configure 8 FDB entries on port\trunk into different port groups;
    trigger aging twice (to get AA messages);
    check that got only single AA message from appropriate port group.
*/
/*
*   a.        Into port group 0 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*       i.        Trigger aging twice. (to get AA messages)
*       ii.        Check that got only single AA message from port group 0.
*   b.        Into port group 1 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*       i.        Trigger aging twice. (to get AA messages)
*       ii.        Check that got only single AA message from port group 1.
*   c.        Into port group 2 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*       i.        Trigger aging twice. (to get AA messages)
*       ii.        Check that got only single AA message from port group 2.
*   d.        ... in the same logic do for port groups 3..6
*   e.        Into port group 7 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*       i.        Trigger aging twice. (to get AA messages)
*       ii.        Check that got only single AA message from port group 7.
*
*   Use next 8 trunks for the test: 0xC,0x1C,0x2C,0x3C,0x4C,0x5C,0x6C,0x7C.
*
*   a.        Into port group 0 : Set 8 FDB entries associated with trunks (as dynamic entry)
*       i.        Trigger aging twice. (to get AA messages)
*       ii.        Check that got 8 AA (for each and every entry).
*   b.        ... in the same logic do for port groups 2..6
*   c.        Into port group 7 : Set 8 FDB entries associated with trunks (as dynamic entry)
*       i.        Trigger aging twice. (to get AA messages)
*       ii.        Check that got 8 AA (for each and every entry).

*/
UTF_TEST_CASE_MAC(tgfFdbAgingInMultiPortGroupDevice)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
   /*
        1. test only for multi port group devices.
    */
    /* this test not valid for Bobcat2; Caelum; Bobcat3 because the Bobcat2; Caelum; Bobcat3 although has port groups
       it has single FDB unit */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* Set configuration */
    prvTgfFdbAgingInMultiPortGroupDeviceConfigurationSet();

    /* Generate traffic */
    prvTgfFdbAgingInMultiPortGroupDeviceTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbAgingInMultiPortGroupDeviceConfigurationRestore();
}

UTF_TEST_CASE_MAC(prvTgfFdbAuMessageFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA3_E);

    /* Set configuration */
    prvTgfFdbAuMessageFilteringConfigurationSet();

    /* Generate traffic */
    prvTgfFdbAuMessageFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbAuMessageFilteringConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IEEE Reserved MC Profiles:
    configure VLAN entry;
    set 802.1Q Bridge, VLAN-aware mode;
    configure IEEE Reserved MC profile;
    enables Trap to the CPU packets with DA in IEEE reserved
    send IPv4 packets on all ports to verify functionality.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Set 802.1Q Bridge, VLAN-aware mode;
    Enable trapping to CPU MC packets;
    Set IEEE Reserved MC profile for all profileIdx [0..3] and all portIdx [0..3];
    Enables Trap to the CPU packets with DA in IEEE reserved, MC addresses
    for bridge control traffic for all profileIdx [0..3 and all protocols [0...255];
    Send IPv4 packet on all portIdx and verify that fisrt packet is not captured
    on portsIdx [0, 1, 2, 3] but is captured in CPU, second packet is dropped.
*/
UTF_TEST_CASE_MAC(tgfBrgGenIeeeReservedMcastProfiles)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* Set configuration */
    prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Port force source ID assignment:
    configure VLAN, FDB entries;
    disable force source-ID assigment on send port;
    delete some port from source-ID group;
    send and verify packet is not captured in all ports;
    enable force source-ID assigment on send port;
    send and verify packet is captured on deleted port.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Create FDB entry on portIdx 0, vlan 2, sourceId 1;
    Create FDB entry on portIdx 1, vlan 2, sourceId 0;
    Disable force source-ID assigment for portIdx 0;
    Set global assign mode to FDB SA-based;
    Set default source-ID to 2 for portIdx 0;
    Delete portIdx 1 from source-ID group 1;
    Enable UC egress filter for portIdx 1;
    Send packet with etherType = 0x8100 vlanID = 0x2 on portIdx 0 and verify
    that packet is not captured in all ports;
    Enable force source-ID assigment for portIdx 0;
    Send packet with etherType = 0x8100 vlanID = 0x2 on portIdx 0 and verify
    that packet is captured on portIdx 1.
*/
UTF_TEST_CASE_MAC(prvTgfBrgSrcIdPortForce)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgSrcIdPortForceConfigurationSet();

    /* Generate traffic */
    prvTgfBrgSrcIdPortForceTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgSrcIdPortForceConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Port source ID assignment when 'bypass ingress pipe'.
    run test prvTgfBrgSrcIdPortForce
*/
UTF_TEST_CASE_MAC(prvTgfBrgSrcIdPortForce_bypassIngressPipe)
{
    prvTgfBrgSrcIdPortPclRedirect(GT_TRUE/*enable PCL redirect*/ , GT_TRUE/*enable bypass ingress pipe*/ );
    UTF_TEST_CALL_MAC(prvTgfBrgSrcIdPortForce);
    prvTgfBrgSrcIdPortPclRedirect(GT_TRUE/*enable PCL redirect*/ , GT_FALSE/*disable bypass ingress pipe*/);
    UTF_TEST_CALL_MAC(prvTgfBrgSrcIdPortForce);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test FDB SA-based source ID assignment:
    configure VLAN, FDB entries;
    disable force source-ID assigment on send port;
    delete some port from source-ID group;
    send and verify packet is not captured in all ports;
    add deleted port to source-ID group;
    send and verify packet is captured on added port.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Create FDB entry on portIdx 0, vlan 2, sourceId 1;
    Create FDB entry on portIdx 1, vlan 2, sourceId 0;
    Disable force source-ID assigment for portIdx 0;
    Set global assign mode to FDB SA-based;
    Set default source-ID to 2 for portIdx 0;
    Delete portIdx 1 from source-ID group 1;
    Enable UC egress filter for portIdx 1;
    Send packet with etherType = 0x8100 vlanID = 0x2 on portIdx 0 and verify
    that packet is not captured in all ports;
    Add to Source-ID group 1 portIdx 1;
    Delete from Source-ID group 2 portIdx 1;
    Send packet with etherType = 0x8100 vlanID = 0x2 on portIdx 0 and verify
    that packet is captured on portIdx 1.
*/
UTF_TEST_CASE_MAC(prvTgfBrgSrcIdFdbSaAssigment)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgSrcIdFdbSaAssigmentConfigurationSet();

    /* Generate traffic */
    prvTgfBrgSrcIdFdbSaAssigmentTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgSrcIdFdbSaAssigmentConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test FDB DA-based source ID assignment:
    configure VLAN, FDB entries;
    disable force source-ID assigment on send port;
    delete some port from source-ID group;
    send and verify packet is not captured in all ports;
    add deleted port to source-ID group;
    delete this port from previous Source-ID group 2;
    send and verify packet is captured on added port.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Create FDB entry on portIdx 0, vlan 2, sourceId 1;
    Create FDB entry on portIdx 1, vlan 2, sourceId 0;
    Disable force source-ID assigment for portIdx 0;
    Set global assign mode to FDB DA-based;
    Set default source-ID to 2 for portIdx 0;
    Delete portIdx 1 from source-ID group 1;
    Enable UC egress filter for portIdx 1;
    Send packet with etherType = 0x8100 vlanID = 0x2 on portIdx 0 and verify
    that packet is not captured in all ports;
    Add to Source-ID group 1 portIdx 1;
    Delete from Source-ID group 2 portIdx 1;
    Send packet with etherType = 0x8100 vlanID = 0x2 on portIdx 0 and verify
    that packet is captured on portIdx 1.
*/
UTF_TEST_CASE_MAC(prvTgfBrgSrcIdFdbDaAssigment)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfBrgSrcIdFdbDaAssigmentConfigurationSet();

    /* Generate traffic */
    prvTgfBrgSrcIdFdbDaAssigmentTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgSrcIdFdbDaAssigmentConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN Tag1 Removal If VID=0 disabled - Ingress packet tagged:
    configure VLAN, FDB entries;
    configure Tunnel, match tagged packed and assigned Tag1 Vlan ID is zero;
    disable Vlan Tag1 removal;
    send tagged traffic and verify that TAG1 is not removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Configure Tunnel, match tagged packed and assigned Tag1 Vlan ID iz zero;
    Disable Vlan Tag1 removal;
    Send tagged UC packet on portIdx 0 and verify that TAG1 is not removed - TAG1
    Vlan is Inner Tag and TAG0 is outer tag.
*/
UTF_TEST_CASE_MAC(prvTgfVlanTag1RemIfZeroDisabled)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_LION_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* The feature is not supported for xCat revision 1,2 */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision < 3)
        {
            PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E);
        }
    }

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* Set configuration */
    prvTgfVlanTag1RemIfZeroConfigurationSet();

    /* Generate traffic */
    prvTgfVlanTag1RemIfZeroDisabledTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanTag1RemIfZeroConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN Tag1 Removal If VID=0 disabled - Ingress packet single tagged:
    configure VLAN, FDB entries;
    configure Tunnel, match tagged packed and assigned Tag1 Vlan ID is zero;
    enable Vlan Tag1 removal for NOT double tagged;
    send tagged traffic and verify Tag1 is removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Configure Tunnel, match tagged packed and assigned Tag1 Vlan ID iz zero;
    Enable Vlan TAG1 removal only if ingress was NOT double tag;
    Send tagged UC packet on portIdx 0 and verivy that packet ingressed as
    double tagged - TAG1 Vlan is Inner Tag and TAG0 is outer tag.
*/
UTF_TEST_CASE_MAC(prvTgfVlanTag1RemIfZeroIngrPktSingleTagged)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_LION_E );

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* The feature is not supported for xCat revision 1,2 */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision < 3)
        {
            PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E);
        }
    }

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* Set configuration */
    prvTgfVlanTag1RemIfZeroConfigurationSet();

    /* Generate traffic */
    prvTgfVlanTag1RemIfZeroSingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanTag1RemIfZeroConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN Tag1 Removal If VID=0 disabled - Ingress packet double tagged:
    configure VLAN, FDB entries;
    configure Tunnel, match tagged packed and assigned Tag1 Vlan ID is zero;
    enable Vlan Tag1 removal regardless of ingress tagging;
    send double tagged traffic and verify Tag1 is removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Configure Tunnel, match tagged packed and assigned Tag1 Vlan ID iz zero;
    Enable Vlan TAG1 removal regardless of ingress tagging;
    Send double tagged packet on portIdx 0 and verivy that TAG1 is removed.
*/
UTF_TEST_CASE_MAC(prvTgfVlanTag1RemIfZeroIngrPktDoubleTagged)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_LION_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* The feature is not supported for xCat revision 1,2 */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision < 3)
        {
            PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E);
        }
    }

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* Set configuration */
    prvTgfVlanTag1RemIfZeroConfigurationSet();

    /* Generate traffic */
    prvTgfVlanTag1RemIfZeroDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanTag1RemIfZeroConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN Tag1 Removal If VID=0 disabled - Ingress packet untagged:
    configure VLAN, FDB entries;
    configure Tunnel, match tagged packed and assigned Tag1 Vlan ID is zero;
    enable Vlan Tag1 removal regardless of ingress tagging;
    send untagged traffic and verify Tag1 is removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Configure Tunnel, match tagged packed and assigned Tag1 Vlan ID iz zero;
    Enable Vlan TAG1 removal regardless of ingress tagging;
    Send untagged packet on portIdx 0 and verivy that TAG1 is removed.
*/
UTF_TEST_CASE_MAC(prvTgfVlanTag1RemIfZeroIngrPktUntagged)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_LION_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* The feature is not supported for xCat revision 1,2 */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision < 3)
        {
            PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E);
        }
    }

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* Set configuration */
    prvTgfVlanTag1RemIfZeroConfigurationSet();

    /* Generate traffic */
    prvTgfVlanTag1RemIfZeroUntagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanTag1RemIfZeroConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN Tag1 Removal If VID=0 enabled for not double tagged packets -
    Ingress packet double tagged:
    configure VLAN, FDB entries;
    configure Tunnel, match tagged packed and assigned Tag1 Vlan ID is zero;
    enable Vlan Tag1 removal only if ingress was NOT double tag;
    send double tagged traffic and verify Tag1 is not removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Configure Tunnel, match tagged packed and assigned Tag1 Vlan ID iz zero;
    Enable Vlan TAG1 removal only if ingress was NOT double tag;
    Send double tagged packet on portIdx 0 and verivy that TAG1 is not removed -
    TAG1 Vlan is Inner Tag and TAG0 is outer tag.
*/
UTF_TEST_CASE_MAC(prvTgfVlanTag1RemIfZeroDoubleTagDisable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_LION_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* The feature is not supported for xCat revision 1,2 */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision < 3)
        {
            PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E);
        }
    }

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* Set configuration */
    prvTgfVlanTag1RemIfZeroConfigurationSet();

    /* Generate traffic */
    prvTgfVlanTag1RemIfZeroDoubleTagDisableTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanTag1RemIfZeroConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN Tag1 Removal If VID=0 enabled - Ingress packet double tagged:
    configure VLAN, FDB entries;
    configure Tunnel, match tagged packed and assigned Tag1 Vlan ID is zero;
    enable Vlan Tag1 removal regardless of ingress tagging;
    send double tagged traffic and verify Tag1 is not removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Create VLANs[portIdx]: 5[0, 1, 2, 3] with tag cmd "Outer TAG1 Inner TAG0";
    Configure Tunnel, match tagged packed and assigned Tag1 Vlan ID iz not zero;
    Enable Vlan TAG1 removal regardless of ingress tagging;
    Send double tagged packet on portIdx 0 and verivy that TAG1 is not zero and
    therefore is not removed - TAG1 Vlan is Inner Tag and TAG0 is outer tag.
*/
UTF_TEST_CASE_MAC(prvTgfVlanTag1RemIfZeroTag1AssignmentIsNotZero)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                        UTF_LION_E );
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* The feature is not supported for xCat revision 1,2 */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision < 3)
        {
            PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E);
        }
    }

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        return;
    }

    /* Set configuration */
    prvTgfVlanTag1RemIfZeroConfigurationSet();

    /* Generate traffic */
    prvTgfVlanTag1RemIfZeroTag1AssignmentIsNotZeroTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanTag1RemIfZeroConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Keep Vlan Tag1 enabled - Tag state is Vlan1&Vlan0 packet ingress with double tag {Tag0, Tag1}:
    configure VLAN, FDB entries;
    enable keeping VLAN1 in the packet;
    send double tagged packet;
    verivy that TAG1 is not removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Outer TAG0 Inner TAG1";
    Enable keeping VLAN1 in the packet for dst portIdx 2 and UP 4;
    Send double tagged packet with TAG0 = 2 and TAG1 = 7 on portIdx 0 and
    verivy that TAG1 is not removed and it's value 7.
*/
UTF_TEST_CASE_MAC(prvTgfVlanKeepVlan1EnTagStateVlan1Vlan0DoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfVlanKeepVlan1ConfigurationSet();

    /* Generate traffic */
    prvTgfVlanKeepVlan1EnTagStateVlan1Vlan0DoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanKeepVlan1ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Keep Vlan Tag1 enabled - Tag state is Vlan0 packet ingress with double tag {Tag0, Tag1}:
    configure VLAN, FDB entries;
    enable keeping VLAN1 in the packet;
    send double tagged packet;
    verivy that TAG1 is not removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "Inner TAG0";
    Enable keeping VLAN1 in the packet for dst portIdx 2 and UP 6;
    Send double tagged packet with TAG0 = 2 and TAG1 = 7 on portIdx 0 and
    verivy that Tag1 is not removed and it's value 7.
*/
UTF_TEST_CASE_MAC(prvTgfVlanKeepVlan1EnTagStateVlan0DoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfVlanKeepVlan1ConfigurationSet();

    /* Generate traffic */
    prvTgfVlanKeepVlan1EnTagStateVlan0DoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanKeepVlan1ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Keep Vlan Tag1 enabled - Tag state is untagged packet ingress with double tag {Tag0, Tag1}:
    configure VLAN, FDB entries;
    enable keeping VLAN1 in the packet;
    send double tagged packet;
    verivy that TAG1 is not removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with untaged cmd;
    Enable keeping VLAN1 in the packet for dst portIdx 2 and UP 4;
    Send double tagged packet with TAG0 = 2 and TAG1 = 7 on portIdx 0 and
    verivy that TAG1 is not removed and it's value 7.
*/
UTF_TEST_CASE_MAC(prvTgfVlanKeepVlan1EnTagStateUntaggedDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfVlanKeepVlan1ConfigurationSet();

    /* Generate traffic */
    prvTgfVlanKeepVlan1EnTagStateUntaggedDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanKeepVlan1ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Keep Vlan Tag1 enabled - Tag state is Vlan0 packet ingress with single tag {Tag0}:
    configure VLAN, FDB entries;
    enable keeping VLAN1 in the packet;
    send tagged packet;
    verivy that TAG1 is removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "inner TAG0";
    Enable keeping VLAN1 in the packet for dst portIdx 2 and UP 6;
    Send tagged packet with TAG0 = 2 on portIdx 0 and verivy that TAG1 is removed.
*/
UTF_TEST_CASE_MAC(prvTgfVlanKeepVlan1EnTagStateVlan0SingleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfVlanKeepVlan1ConfigurationSet();

    /* Generate traffic */
    prvTgfVlanKeepVlan1EnTagStateVlan0SingleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanKeepVlan1ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Keep Vlan Tag1 disabled - Tag state is Vlan0 packet ingress with single tag {Tag0}:
    configure VLAN, FDB entries;
    disable keeping VLAN1 in the packet;
    send double tagged packet;
    verivy that TAG1 is removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with tag cmd "inner TAG0";
    Disable keeping VLAN1 in the packet for dst portIdx 2 and UP 4;
    Send double tagged packet with TAG0 = 2 and TAG1 = 7 on portIdx 0 and
    verivy that TAG1 is removed.
*/
UTF_TEST_CASE_MAC(prvTgfVlanKeepVlan1DisTagStateVlan0DoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfVlanKeepVlan1ConfigurationSet();

    /* Generate traffic */
    prvTgfVlanKeepVlan1DisTagStateVlan0DoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanKeepVlan1ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Keep Vlan Tag1 disabled - Tag state is untagged packet ingress untagged:
    configure VLAN, FDB entries;
    disable keeping VLAN1 in the packet;
    send double tagged packet;
    verivy that TAG1 is removed.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] with untagged cmd;
    Disable keeping VLAN1 in the packet for dst portIdx 2 and UP 4;
    Send double tagged packet with TAG0 = 2 and TAG1 = 7 on portIdx 0 and
    verivy that TAG1 is removed.
*/
UTF_TEST_CASE_MAC(prvTgfVlanKeepVlan1DisTagStateUntaggedDoubleTag)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfVlanKeepVlan1ConfigurationSet();

    /* Generate traffic */
    prvTgfVlanKeepVlan1DisTagStateUntaggedDoubleTagTrafficGenerate();

    /* Restore configuration */
    prvTgfVlanKeepVlan1ConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IEEE Reserved MC Learning - enabled, cmd Trap to CPU:
    configure VLAN, FDB entries;
    enable IEEE Reserved MC Learning for trapped packets;
    enable trap to CPU for IEEE Reserved MC packets;
    send tagged packet 0 and verivy it is trapped to CPU and Learned on FDB.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Set default pvid 2 for portIdx 3;
    Enable IEEE Reserved MC Learning for trapped packets;
    Set Profile index 2 for portIdx 1;
    Set command TRAP_TO_CPU for profile index 2 and protocol 0x55;
    Enable trap to CPU for IEEE Reserved MC packets;
    Send tagged packet on portIdx 0 and verivy that packet is trapped to CPU
    and Learned on FDB.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenIeeeReservedMcastLearningEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);



    /* Set configuration */
    prvTgfBrgGenIeeeReservedMcastLearningConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenIeeeReservedMcastLearningEnableTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenIeeeReservedMcastLearningConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IEEE Reserved MC Learning - disabled, cmd Trap to CPU:
    configure VLAN, FDB entries;
    disable IEEE Reserved MC Learning for trapped packets;
    enable trap to CPU for IEEE Reserved MC packets;
    send tagged packet 0 and verivy it is trapped to CPU and not Learned on FDB.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Set default pvid 2 for portIdx 3;
    Disable IEEE Reserved MC Learning for trapped packets;
    Set Profile index 2 for portIdx 1;
    Set command TRAP_TO_CPU for profile index 2 and protocol 0x55;
    Enable trap to CPU for IEEE Reserved MC packets;
    Send tagged packet on portIdx 0 and verivy that packet is trapped to CPU
    and not Learned on FDB.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenIeeeReservedMcastLearningDisable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgGenIeeeReservedMcastLearningConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenIeeeReservedMcastLearningDisableTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenIeeeReservedMcastLearningConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IEEE Reserved MC Learning - disabled, cmd Forward:
    configure VLAN, FDB entries;
    disable IEEE Reserved MC Learning for trapped packets;
    disable trap to CPU for IEEE Reserved MC packets;
    send tagged packet 0 and verivy it is not trapped to CPU and Learned on FDB.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Set default pvid 2 for portIdx 3;
    Disable IEEE Reserved MC Learning for trapped packets;
    Set Profile index 2 for portIdx 1;
    Set command FORWARD for profile index 2 and protocol 0x55;
    Disable trap to CPU for IEEE Reserved MC packets;
    Send tagged packet on portIdx 0 and verivy that packet is not trapped to CPU
    and Learned on FDB.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenIeeeReservedMcastLearningDisableTrapDisable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgGenIeeeReservedMcastLearningConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenIeeeReservedMcastLearningDisableTrapDisableTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenIeeeReservedMcastLearningConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test enable\disable Rate Limit on ingress port for MC registered\unregistered packets:
    configure VLAN, FDB, VIDX entries;
    enable Rate Limit for MC registered packets;
    disable Rate Limit for MC unregistered packets;
    config window size, rate limit and drop command;
    send traffic and verify that one packet is received on configured ports and one packet is dropped.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Enable Rate Limit on portIdx 2 for MC registered packets;
    Disable Rate Limit on portIdx 2 for MC unregistered packets;
    Create vidx 0xF with port members portIdx 0 and portIdx 1;
    Create FDB entry on vidx 0xF, vlan 5;
    Set window size to maximum and drop command to hard;
    Set rate limit to 1;
    Send tagged packet on portIdx 2 and verivy that one packet is received on
    portIdx 0 and portIdx 1 and one packet is dropped.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenRateLimitMcastRegEnableRegPkts)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* the GM devices not support rate-limit features , and not clear the 'counted' packets in the window size .*/
    /* in GM - this test kills the MC traffic during the test ,and it is not recovering ! */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenRateLimitMcastRegEnableRegTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test disable Rate Limit on ingress port for MC registered\unregistered packets:
    configure VLAN, FDB, VIDX entries;
    disable Rate Limit for MC registered and unregistered packets;
    config window size, rate limit and drop command;
    send traffic and verify that two packets are received on config ports and no packet is dropped.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Disable Rate Limit on portIdx 2 for MC registered and unregistered packets;
    Create vidx 0xF with port members portIdx 0 and portIdx 1;
    Create FDB entry on vidx 0xF, vlan 5;
    Set window size to maximum and drop command to hard;
    Set rate limit to 1;
    Send tagged packet on portIdx 2 and verivy that two packet are received on
    portIdx 0 and portIdx 1 and no packet is dropped.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenRateLimitMcastRegDisableRegPkts)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* the GM devices not support rate-limit features , and not clear the 'counted' packets in the window size .*/
    /* in GM - this test kills the MC traffic during the test ,and it is not recovering ! */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenRateLimitMcastRegDisableRegTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test enable\disable Rate Limit on ingress port for MC registered\unregistered packets:
    configure VLAN, FDB, VIDX entries;
    enable Rate Limit for MC registered packets;
    disable Rate Limit for MC unregistered packets;
    config window size, rate limit and drop command;
    send unregistered MC traffic and verify that two packet are received on config and no packet is dropped.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Enable Rate Limit on portIdx 2 for MC registered packets;
    Disable Rate Limit on portIdx 2 for MC unregistered packets;
    Create vidx 0xF with port members portIdx 0 and portIdx 1;
    Create FDB entry on vidx 0xF, vlan 5;
    Set window size to maximum and drop command to hard;
    Set rate limit to 1;
    Send unregistered MC tagged packet on portIdx 2 and verivy that two packet
    are received on portIdx 0 and portIdx 1 and no packet is dropped.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenRateLimitMcastRegEnableUnregPkts)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* the GM devices not support rate-limit features , and not clear the 'counted' packets in the window size .*/
    /* in GM - this test kills the MC traffic during the test ,and it is not recovering ! */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenRateLimitMcastRegEnableUnregTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test disable\enable Rate Limit on ingress port for MC registered\unregistered packets:
    configure VLAN, FDB, VIDX entries;
    disable Rate Limit for MC registered packets;
    enable Rate Limit for MC unregistered packets;
    config window size, rate limit and drop command;
    send traffic and verify that two packet is received on config ports and no packet is dropped.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Disable Rate Limit on portIdx 2 for MC registered packets;
    Enable Rate Limit on portIdx 2 for MC unregistered packets;
    Create vidx 0xF with port members portIdx 0 and portIdx 1;
    Create FDB entry on vidx 0xF, vlan 5;
    Set window size to maximum and drop command to hard;
    Set rate limit to 1;
    Send tagged packet on portIdx 2 and verivy that two packet is received on
    portIdx 0 and portIdx 1 and no packet is dropped.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenRateLimitMcastRegDisableUnregEnableRegPkts)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* the GM devices not support rate-limit features , and not clear the 'counted' packets in the window size .*/
    /* in GM - this test kills the MC traffic during the test ,and it is not recovering ! */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationSet();

    /* Generate traffic */
    prvTgfBrgGenRateLimitMcastRegDisableUnregEnableRegTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgGenRateLimitMcastRegConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Disable learning:
    configure VLAN, disable learning;
    send unknown UC tagged traffic on all ports;
    verify entry offset in AU NA message is updated correctly.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3]
    Disable learning;
    Send tagged broadcast packets with different hash entry offsets on portIdx and verivy that
    AU NA message entry offset is updated correctly :
    entryOffset.
*/
UTF_TEST_CASE_MAC(prvTgfFdbAuNaMessageExtFormatFdbHashIndex)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_DXCH_E);

    /* Set configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationSet();

    /* Generate traffic */
    prvTgfFdbAuNaMessageExtFormatWithFdbBankIndex();

    /* Restore configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test check spUnknown in the NA message format
*/
/*
    1. disable NA processing at the appDemo
    2. use default vlan.
    3. set SP on ingress port
    4. send packet and check spUnknown = GT_TRUE at the NA
    5. send packet and check no new NA !
    6. flush FDB
    7. set 'no' SP on ingress port
    8. send packet and check spUnknown = GT_FALSE at the NA
    5. send packet and check new NA !
    9. disable NA processing at the appDemo
    10. flush FDB

*/
UTF_TEST_CASE_MAC(prvTgfFdbAuNaMessageSpUnknown)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_DXCH_E);

    /* Set configuration */
    prvTgfFdbAuNaMessageConfigurationSet(TESTED_FIELD_IN_NA_MSG_SP_UNKNOWN_E);

    /* Generate traffic */
    prvTgfFdbAuNaMessageTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbAuNaMessageConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Disable learning:
    configure VLAN, FDB entries;
    disable learning;
    send unknown UC double tagged traffic on all ports;
    verify AU NA message is updated correctly including extended parameters.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3], 3[0, 1, 2, 3];
    Create VLANs[portIdx]: 5[0, 1, 2, 3], 4094[0, 1, 2, 3];
    Create FDB entries on portIdx [0, 1, 2, 3], vlan 2;
    Disable learning;
    Send double tagged packets with unknown UC on all portIdx and verivy that
    AU NA message is updated correctly including the extended parameters:
    vid1, up0 and isMoved.
*/
UTF_TEST_CASE_MAC(prvTgfFdbAuNaMessageExtFormatWithTag1DueToNewAddr)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationSet();

    /* Generate traffic */
    prvTgfFdbAuNaMessageExtFormatWithTag1DueToNewAddrTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Learn MAC on FDB on port:
    configure VLAN, FDB entries;
    disable learning;
    send known UC tagged traffic on all ports;
    verify AU NA message is updated correctly including extended parameters.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Create FDB entry on portIdx [0, 1, 2, 3], vlan 2;
    Disable learning;
    Send tagged packet with known UC on all portIdx and verivy that
    AU NA message is updated correctly including the extended parameters:
    up0, isMoved oldSrcId, oldDstInterface and oldAssociatedDevNum.
*/
UTF_TEST_CASE_MAC(prvTgfFdbAuNaMessageExtFormatDueToMovedPort)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationSet();

    /* Generate traffic */
    prvTgfFdbAuNaMessageExtFormatDueToMovedPortTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Learn MAC on FDB on trunk:
    configure VLAN, FDB entries;
    disable learning;
    send known UC tagged traffic on all ports;
    verify AU NA message is updated correctly including extended parameters.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Create FDB entries on trunk [23, 127, 1, 58], vlan 2;
    Disable learning;
    Send tagged packet with known UC on all portIdx and verivy that
    AU NA message is updated correctly including the extended parameters:
    up0, isMoved oldSrcId, oldDstInterface and oldAssociatedDevNum.
*/
UTF_TEST_CASE_MAC(prvTgfFdbAuNaMessageExtFormatDueToMovedTrunk)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);


{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    /* Set configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationSet();

    /* Generate traffic */
    prvTgfFdbAuNaMessageExtFormatDueToMovedTrunkTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbAuNaMessageExtFormatConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Acceptable frame type:
    configure VLAN, FDB entries;
    configure drop counter according to acceptable frame type;
    set specific acceptable frame type for each port;
    send UC tagged\untagged traffic and verify drop counters.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3];
    Set pvid 2 for all portIdx;
    Set drop counter to count only packets dropped due acceptable frame type;
    Set specific acceptable frame type for each portIdx;
    Send UC tagged and untagged traffic on portIdx 0, 1 and verify that
    drop counters and ports counters is correct.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanPortAccFrameType)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfBrgVlanPortAccFrameTypeConfigSet();

    /* Generate traffic */
    prvTgfBrgVlanPortAccFrameTypeTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanPortAccFrameTypeConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Protocol-Based VLAN LLC/Non-SNAP:
    configure VLAN, FDB entries;
    config for all packets port protocol VID\QOS assignment;
    set non-SNAP LLC encapsulation of Protocol based classification;
    send tagged\untagged traffic 1 and verify expected traffic.
*/
/*
    Create VLANs[portIdx]: 5[0, 1, 2] and 6[0, 2];
    Enable port protocol VLAN assignment on portIdx 0;
    Assign for all packets port protocol VID\QOS assignment on portIdx 0;
    Set non-SNAP LLC encapsulation of Protocol based classification;
    Send tagged and untagged traffic on portIdx 0, 1 and verify expected traffic.
*/
UTF_TEST_CASE_MAC(prvTgfBrgProtocolBasedVlanLlc)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Set configuration */
    prvTgfBrgProtocolBasedVlanLlcConfigSet();

    /* Generate traffic */
    prvTgfBrgProtocolBasedVlanLlcTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgProtocolBasedVlanLlcConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Invalid VLAN filtering diff vlans:
    configure VLAN, FDB entries;
    invalidate one VLAN entry;
    send traffic on invalidated VLAN and verify traffic on other VLAN.
*/
/*
    Create VLANs[portIdx]: 2[0, 1] and 3[2, 3];
    Invalidate VLAN 2 entry;
    Send tagged traffic on portIdx 0, 2 and verify expected traffic on VLAN 3.
*/
UTF_TEST_CASE_MAC(prvTgfBrgInvalidVlanFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);


    /* Set configuration */
    prvTgfBrgInvalidVlanFilteringConfigSet();

    /* Generate traffic */
    prvTgfBrgInvalidVlanFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgInvalidVlanFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Egress Tag Modification on Non-DSA-Tagged Egress Port:
    configure VLAN, FDB entries;
    enable FDB LSR operations;
    send tagged\untagged traffic different DA;
    verify tagged packets from tagged port and untagged packets from untagged ports.
*/
/*
    Create VLANs[portIdx]: 2[0, 1, 2, 3] and tag cmd mask [1, 1, 1, 0];
    Enable FDB Learning, Scanning and Refresh operations;
    Set Independent VLAN Learning Lookup mode;
    Create 2 FDB entry on portIdx 2, 3 vlan 2;
    Set PVID 2 on portIdx 1;
    Send tagged and untagged traffic with different DA on portIdx 2 and verify
    tagged packets from tagged port and untagged packets from untagged ports.
*/
UTF_TEST_CASE_MAC(prvTgfBrgEgrTagModifNonDsaTaggedEgrPort)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Set configuration */
    prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigSet();

    /* Generate traffic */
    prvTgfBrgEgrTagModifNonDsaTaggedEgrPortTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Nested customer to core:
    configure VLAN, FDB entries;
    sets egress S-VLAN ether type;
    set nested VLAN config;
    config customer untagged port and tagged core port;
    send tagged traffic to customer port;
    verify double tagged traffic on core port.
*/
/*
    Create VLANs[portIdx]: 3[0, 2] and tag cmd mask [1, 0];
    Set pvid 3 on portIdx 2;
    Sets egress S-VLAN ether type;
    Sets VLAN EtherType for tagged packets transmitted via egress portIdx 0;
    Set nested VLAN customer port on portIdx 0;
    Set nested VLAN access port on portIdx 2;
    Configure customer untagged port and tagged core port;
    Send tagged traffic to customer port and expect double tagged traffic on
    core port.
*/
UTF_TEST_CASE_MAC(prvTgfBrgNestedCustomerToCore)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgNestedCustomerToCoreConfigSet();

    /* Generate traffic */
    prvTgfBrgNestedCustomerToCoreTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgNestedCustomerToCoreConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Nested customer to core DSA egress port:
    configure VLAN, FDB entries;
    sets egress S-VLAN ether type;
    set nested VLAN config;
    config customer untagged port and tagged core port;
    send tagged traffic to customer port;
    verify double tagged traffic on core port.
*/
/*
    Create VLANs[portIdx]: 3[0, 2] and tag cmd mask [1, 0];
    Set pvid 3 on portIdx 2;
    Sets egress S-VLAN ether type;
    Sets VLAN EtherType for tagged packets transmitted via egress portIdx 0;
    Set nested VLAN customer port on portIdx 0;
    Set nested VLAN access port on portIdx 2;
    Configure customer untagged port and tagged core port;
    Send VLAN tagged traffic to customer port and expect DSA tagged traffic on
    core port.
*/
UTF_TEST_CASE_MAC(prvTgfBrgNestedCustomerToCoreEgressCascadePort)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    /* enable substitution of VLAN tag with DSA tag on cascade egress port */
    prvTgfBrgNestedCustomerToCoreTestAsDsa(TGF_DSA_2_WORD_TYPE_E);

    /* Set configuration */
    prvTgfBrgNestedCustomerToCoreConfigSet();

    /* Generate traffic */
    prvTgfBrgNestedCustomerToCoreEgressCascadePortTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgNestedCustomerToCoreConfigRestore();

    /* enable substitution of VLAN tag with eDSA tag on cascade egress port */
    prvTgfBrgNestedCustomerToCoreTestAsDsa(TGF_DSA_4_WORD_TYPE_E);

    /* Set configuration */
    prvTgfBrgNestedCustomerToCoreConfigSet();

    /* Generate traffic */
    prvTgfBrgNestedCustomerToCoreEgressCascadePortTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgNestedCustomerToCoreConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test rate limit on stack ports under WS traffic:
    configure VLAN, FDB, VIDX entries;
    send registered MC traffic;
    verify traffic rate on egress ports is same as on ingress one.
*/
/*
    Create VLANs[portIdx]: 245[0, 1], 439[2, 3];
    Create MC Group VIDX[portIdx]: 33[0, 1], 22[2, 3];
    Create FDB entries on vidx 33, vlan 245 and on vidx 22, vlan 439;
    Send registered MC traffic on portIdx 0 and 1 and verify that traffic rate
    on egress ports is same as rate limit on ingress one.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenRateLimitStackPortWsTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_PUMA_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* execute test */
    prvTgfBrgGenRateLimitWsTest();
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test FDB entry interface type functionality (port, trunk, vid, vidx):
    configure VLAN, FDB, VIDX, TRUNK entries;
    configure FDB entries on Port\VID\TRUNK\VIDX interfaces;
    send traffic on each interface to verify FDB entry.
*/
/*
    Create vlan with vlanId 5 and all untagged portIdxs [0, 1, 2, 3];
    Create MC group with vidx 1 and portIdxs [0, 1, 2, 3];
    Create trunk with trunkId 2 and add portIdxs members [1, 2];

    Create FDB entry on port with MAC as in sent packet DA, vlanId 5 and portIdx 0;
    Send traffic on portIdx 1
    Check results;

    Create FDB entry on VID with MAC as in sent packet DA, vlanId 5;
    Send traffic on portIdx 0;
    Check results;

    Create FDB entry on Trunk with MAC as in sent packet DA, trunkId 2;
    Send traffic on portIdx 0;
    Check results;

    Create FDB entry on Vidx with MAC as in sent packet DA, Vidx = 1;
    Send traffic on portIdx 0;
    Check results;
*/
UTF_TEST_CASE_MAC(prvTgfFdbChangeInterfaceType)
{
/********************************************************************
    Test - Change Interface Type (port, trunk, vid, vidx)
           FDB entry can be defined as Port entry, Trunk entry,
           Vid entry & Vidx entry. The test verifies that all
           interface types work properly.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    /* Set configuration */
    prvTgfFdbChangeInterfaceTypeConfigSet();

    /* Generate traffic */
    prvTgfFdbChangeInterfaceTypeTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbChangeInterfaceTypeConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test FDB DA command:
    configure VLAN, FDB entries;
    configure FDB entries with different DA commands;
    send traffic on each interface to verify commands.
*/
/*
    Set default vlan entry 2

    Iterate thru next stages by change DA command from sequence
    [ PRV_TGF_PACKET_CMD_DROP_HARD_E, PRV_TGF_PACKET_CMD_FORWARD_E,
      PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E, PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E ]
    {
        - Create FDB entry on portIdx 1,
           entryType PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E,
           vlan 2,  macAddr 00:01,
           saCommand PRV_TGF_PACKET_CMD_FORWARD_E,
           dstInterfaceType CPSS_INTERFACE_PORT_E;
        - Select current DA command from sequence list;
        - Send traffic on portIdx 0, dstMAC 00:01, srcMAC 11:11;
        - Check counters;
        - Flush FDB entry;
     }

*/
UTF_TEST_CASE_MAC(prvTgfFdbChangeDaCommand)
{
/********************************************************************
    Test - Change DA Command. DA command can be defined as Drop,
           Forward, Mirror2CPU, Trap2CPU. The test verifies that
           all options of DA Command work properly.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    /* Set configuration */
    prvTgfFdbChangeDaCommandConfigSet();

    /* Generate traffic */
    prvTgfFdbChangeDaCommandTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbChangeDaCommandConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test FDB SA command:
    configure VLAN, FDB entries;
    configure FDB entries with different SA commands;
    send traffic on each interface to verify commands.
*/
/*
    Set default vlan entry 2

    Iterate thru next stages by change SA command from sequence
    [ PRV_TGF_PACKET_CMD_DROP_HARD_E, PRV_TGF_PACKET_CMD_FORWARD_E,
      PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E, PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E ]
    {
        - Create FDB entry on portIdx 1,
           entryType PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E,
           vlan 2,  macAddr 00:02,
           daCommand PRV_TGF_PACKET_CMD_FORWARD_E,
           dstInterfaceType CPSS_INTERFACE_PORT_E;
        - Select current SA command from sequence list;
        - Send traffic on portIdx 0, dstMAC 00:01, srcMAC 00:02;
        - Check counters;
        - Flush FDB entry;
    }
*/
UTF_TEST_CASE_MAC(prvTgfFdbChangeSaCommand)
{
/********************************************************************
    Test - Change SA Command. SA command can be defined as Drop,
           Forward, Mirror2CPU, Trap2CPU. The test verifies that
           all options of SA Command work properly.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    /* Set configuration */
    prvTgfFdbChangeSaCommandConfigSet();

    /* Generate traffic */
    prvTgfFdbChangeSaCommandTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbChangeSaCommandConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify entries aging:
    configure VLAN, FDB entries, FDB table trigger mode;
    add different static\dynamic MAC entries;
    starts triggered action - age with removal;
    send traffic with incremental DA mac address;
    verify counters and FDB entries.
*/
/*
    Set default vlan entry 1;
    Set FDB table trigger mode: action is done via trigger from CPU;
    Enable deleting static entries;
    Add 4th static MAC entries with macAddr from 00:01 to 00:04, on portsIdx from
     0 to 3, all entries aged;
    Add 4th dynamic MAC entries with macAddr from 00:05 to 00:08, on portsIdx
     from 0 to 3, all entries aged;
    Check all new created entries in FDB table by macEntryKey search ( by entryType,
     vlan, macAddr);
    Starts triggered action - age with removal;
    Verify all 8 entries;
    Send traffic with incremental DA mac address (that start from 00:01),
     burst count 8, SA mac addr 0x11:0x11, dev 0, portIdx 0;
    Check counters;
    Starts triggered action - age with removal;
    Verify all 5 entries;
    Send traffic with incremental DA mac address (that start from 00:01),
     burst count 8, SA mac addr 0x11:0x11, dev 0, portIdx 0;
    Check counters;
*/
UTF_TEST_CASE_MAC(prvTgfFdbTriggeredAgingEntries)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFdbTriggeredAgingEntriesConfigSet();

    /* Generate traffic */
    prvTgfFdbTriggeredAgingEntriesTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbTriggeredAgingEntriesConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify group of vlans aging:
    configure VLAN, FDB entries, FDB table trigger mode;
    add different static\dynamic MAC entries;
    starts triggered action - age with removal;
    send traffic with incremental DA mac address;
    verify counters and FDB entries.
*/
/*
    Create Vlan 5 and 13 on ports with indexes 0, 1, 2, 3;
    Set the Vlan lookup mode CPSS_IVL_E;
    Set FDB table trigger mode - action is done via trigger from CPU;
    Sets the device table of the PP with devTableMask 0xFFFF;
    Add 12th static MAC entries with macAddr from 00:01 to 00:04
    thru portsIdx from 0 to 3 on vlan 1, 5, 13;
    Add 12th dynamic MAC entries with macAddr from 00:0x11 to 00:0x14
    thru portsIdx from 0 to 3  on vlan 1, 5, 13;
    Set active vlan: vlanId 5, vlanMask 5;
    Starts triggered action age with removal twice;
    Verify all 12 FDB mac entries;
    Send traffic with incremental DA mac address (that starts from 00:0x11),
    vlanId 1, portIdx 0;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:0x11),
    vlanId 5, portIdx 3;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:0x11),
    vlanId 13, portIdx 0;
    Check counters;

    Send traffic with incremental DA mac address (that starts from 00:01),
    vlanId 1, portIdx 0;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:01),
    vlanId 5, portIdx 3;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:01),
    vlanId 13, portIdx 0;
    Check counters;
*/
UTF_TEST_CASE_MAC(prvTgfFdbTriggeredAgingVlans)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFdbTriggeredAgingVlansConfigSet();

    /* Generate traffic */
    prvTgfFdbTriggeredAgingVlansTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbTriggeredAgingVlansConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify restrict deleting to a specific port + vlan:
    configure VLAN, FDB entries, VLAN lookup mode;
    add different static\dynamic MAC entries;
    starts triggered action - deleting;
    verify deleting static\dynamic entries;
    send traffic with incremental DA mac address on different interfaces;
    verify counters and FDB entries.
*/
/*
    Create Vlan 5 and 13 on ports with indexes 0, 1, 2, 3;
    Set the Vlan lookup mode CPSS_IVL_E;
    Add 12th static MAC entries with macAddr from 00:01 to 00:04
    thru portsIdx from 0 to 3 on vlan 1, 5, 13;
    Add 12th dynamic MAC entries with macAddr from 00:0x11 to 00:0x14
    thru portsIdx from 0 to 3  on vlan 1, 5, 13;
    Set active vlan: vlanId 1, vlanMask 4095;
    Set active interface: devNum 0, actIsTrunk 0, actIsTrunkMask 0,
    actTrunkPort 8, actTrunkPortMask 4095 ;
    Start triggered action - deleting;
    Verify deleting dynamic entry with vlanId 1 port 8;
    Enable delete on static entries;
    Start triggered action - deleting;
    Verify deleting static entry with vlanId 1 port 8;
    Send traffic with incremental DA mac address (that starts from 00:0x11),
    vlanId 1, portIdx 0;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:0x11),
    vlanId 5, portIdx 3;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:0x11),
    vlanId 13, portIdx 0;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:01),
    vlanId 1, portIdx 0;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:01),
    vlanId 5, portIdx 3;
    Check counters;
    Send traffic with incremental DA mac address (that starts from 00:01),
    vlanId 13, portIdx 0;
    Check counters;
*/
UTF_TEST_CASE_MAC(prvTgfFdbDelMacEntriesForVlanPort)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFdbDelMacEntriesForVlanPortConfigSet();

    /* Generate traffic */
    prvTgfFdbDelMacEntriesForVlanPortTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbDelMacEntriesForVlanPortConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Check functionality of Mc Group mirroring:
    configure VLAN, FDB, VIDX entries;
    define Mirror and Analyzer ports;
    send traffic to MC Group;
    check that analyzer port receives the packets too.
*/
/*
    Create MC groups.
    Define Mirror and Analyzer ports.
    Send Packets to MC Group.
    Check that Analyzer port receives the packets too.

    Create Vlan 2 on all ports;
    Create McGroup on vidx 1 with port members 0 and 18;
    Create Fdb macEntry with MAC 01:02:03:04:05:06 port 0, vlanId 2, vidx 1;
    Enable mirroring per port 23;
    Set ingress mirror type on port 8;
    Send traffic on port 23 vlanId 2, DA 01:02:03:04:05:06, SA 00:00:11:11:11:11;
    Check expected traffic on all ports;
*/
UTF_TEST_CASE_MAC(prvTgfMcGroupRxMirror)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* Set configuration */
    prvTgfMcGroupRxMirrorConfigSet();

    /* Generate traffic */
    prvTgfMcGroupRxMirrorTrafficGenerate();

    /* Restore configuration */
    prvTgfMcGroupRxMirrorConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify srcID egress filtering and default srcID assignment mode:
    configure VLAN, FDB entries;
    define default srcId and srcId assignment mode;
    delete\add some port from srcIdGroups;
    enable srcID egress filtering on deleted port;
    send traffic and verify it on configured port.
*/
/*
    Create Vlan 2 on all ports;
    Add MAC entry with macAddr 00:01 on portIdx 0, vlanId 2 with sourceId 1;
    Add MAC entry with macAddr 00:02 on portIdx 1, vlanId 2 with sourceId 0;

    Set source ID assignment mode PORT_DEFAULT on portIdx 0;
    Set default source ID 2 for portIdx 0;
    Delete portIdx 1 from sourceIdGroup 1;
    Enable source ID egress filter for unicast packets on portIdx 1;
    Send traffic on portIdx 0 vlanId 2, SA 00:01, DA 00:02;
    Check expected traffic;

    Delete portIdx 1 from sourceIdGroup 2;
    Add portIdx 1 to  sourceIdGroup 1;
    Send traffic on portIdx 0 vlanId 2, SA 00:01, DA 00:02;
    Check expected traffic;
*/
UTF_TEST_CASE_MAC(prvTgfFdbSourceIdModeDefault)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFdbSourceIdModeDefaultConfigSet();

    /* Generate traffic */
    prvTgfFdbSourceIdModeDefaultTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbSourceIdModeDefaultConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify srcID egress filtering and changed srcID assignment mode:
    configure VLAN, FDB entries with different srcId;
    define default srcId and srcId assignment mode;
    delete\add some port from srcIdGroups;
    enable srcID egress filtering on deleted port;
    send traffic and verify it on configured port.
    change other srcId assignment mode;
    send traffic and verify it on configured port.
*/
/*
    Create Vlan 2 on all ports;
    Add MAC entry with macAddr 00:01 on portIdx 0, vlanId 2 with sourceId 1;
    Add MAC entry with macAddr 00:02 on portIdx 1, vlanId 2 with sourceId 2;
    Add MAC entry with macAddr 00:03 on portIdx 2, vlanId 2 with sourceId 3;
    Add MAC entry with macAddr 00:04 on portIdx 3, vlanId 2 with sourceId 4;

    Set Source ID Assignment mode FDB_PORT_DEFAULT on portIdx 0 and 3

    Configure default source ID 2 for portIdx 0;
    Configure default source ID 4 for portIdx 3;
    Delete portIdx 1 from sourceIdGroup 1;
    Delete portIdx 3 from sourceIdGroup 4;
    Enable source ID egress filter for unicast packets on portIdx 1;
    Enable source ID egress filter for unicast packets on portIdx 3;

    Send traffic on portIdx 0 with vlanId 2, SA 00:01, DA 00:02;
    Send traffic on portIdx 3 with vlanId 2, SA 00:03, DA 00:04;
    Check expected traffic - egress Port shouldn't receive any packet;

    Set source ID assignment mode PORT_DEFAULT on portIdx 0 and 2;
    Send traffic on portIdx 0 and 2 with vlanId 2, SA 00:01, DA 00:02;
    Check expected traffic - egress Port shouldn't receive any packet;
*/
UTF_TEST_CASE_MAC(prvTgfFdbSourceIdModeChange)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

    /* Set configuration */
    prvTgfFdbSourceIdModeChangeConfigSet();

    /* Generate traffic */
    prvTgfFdbSourceIdModeChangeTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbSourceIdModeChangeConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Customer BPDUs - Change Command:
    delete default VLAN, configure VLAN, FDB entries;
    enables trap\mirror to CPU MC packets in IEEE reserved MC range;
    binds port to one of IEEE reserved MC profile;
    set different command for specific IEEE reserved MC protocol;
    send and verify traffic in IEEE reserved MC range.
*/
/*
    Delete default VLAN 1. Create VLAN 2 with tagget all test
    ports. Enables trapping or mirroring to CPU Multicast
    packets per port with MAC_DA in the IEEE reserved
    Multicast range. Binds the port to one of the IEEE reserved
    multicast profiles.

    Iterate thru next stages by change the command for specific
    IEEE reserved multicast protocol at specific profile
    [ CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
    CPSS_PACKET_CMD_TRAP_TO_CPU_E, CPSS_PACKET_CMD_DROP_SOFT_E ]
    {
        - Send traffic on portIdx 0 with dstMAC 01:80:C2:00:00:02;
        - Check counters;
    }
*/
UTF_TEST_CASE_MAC(prvTgfBrgCustomerBpdus)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgCustomerBpdusConfigSet();

    /* Generate traffic */
    prvTgfBrgCustomerBpdusTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgCustomerBpdusConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Provider BPDUs - Change Command:
    delete default VLAN, configure VLAN, FDB entries;
    enables trap\mirror to CPU MC packets in IEEE reserved MC range;
    binds port to one of IEEE reserved MC profile;
    set different command for specific IEEE reserved MC protocol;
    send and verify traffic in IEEE reserved MC range.
*/
/*
    Delete default VLAN 1. Create VLAN 2 with tagget all test
    ports. Enables trapping or mirroring to CPU Multicast
    packets per port with MAC_DA in the IEEE reserved
    Multicast range. Binds the port to one of the IEEE reserved
    multicast profiles.

    Iterate thru next stages by change the command for specific
    IEEE reserved multicast protocol at specific profile
    [ CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
    CPSS_PACKET_CMD_TRAP_TO_CPU_E, CPSS_PACKET_CMD_DROP_SOFT_E ]
    {
        - Send traffic on portIdx 0 with dstMAC 01:80:C2:00:00:22;
        - Check counters;
    }
*/
UTF_TEST_CASE_MAC(prvTgfBrgProviderBpdus)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgProviderBpdusConfigSet();

    /* Generate traffic */
    prvTgfBrgProviderBpdusTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgProviderBpdusConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Learning Addresses in SVL Mode:
    create Vlan 5 and 13;
    set the VLAN Lookup mode to CPSS_SVL_E - Only MAC is used for MAC table lookup and learning;
    send packets with vlan tags 1, 5, 13;
    check that FDB table consists only records with vlanId 1.
*/
UTF_TEST_CASE_MAC(prvTgfFdbLearningAdressesSvlMode)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfFdbLearningAdressesSvlModeConfigSet();

    /* Generate traffic */
    prvTgfFdbLearningAdressesSvlModeTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbLearningAdressesSvlModeConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv4 MC Bridging:
    configure VLAN, FDB, VIDX entries;
    check that packets with MAC DA matching the management
    Unicast MAC Address are forward to the CPU with the CPU code
    CPSS_NET_BRIDGED_PACKET_FORWARD_E.
*/
/*
    However, if the FDB entry <Application Specific CPU Code Enable>
    is set, the CPU code may be overridden b    y a finer-grain CPU code,
    e.g., if the packet is an ARP reply, its CPU code assignment can be
    overridden with the specific CPU code CPSS_NET_ARP_REPLY_TO_ME_E
*/
UTF_TEST_CASE_MAC(prvTgfMcBridgingIpV4)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfMcBridgingIpV4ConfigSet();

    /* Generate traffic */
    prvTgfMcBridgingIpV4TrafficGenerate();

    /* Restore configuration */
    prvTgfMcBridgingIpV4ConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test IPv6 MC Bridging:
    configure VLAN, FDB, VIDX entries;
    check that packets with MAC DA matching the management
    Unicast MAC Address are forward to the CPU with the CPU code
    CPSS_NET_BRIDGED_PACKET_FORWARD_E.
*/
/*
    However, if the FDB entry <Application Specific CPU Code Enable>
    is set, the CPU code may be overridden by a finer-grain CPU code,
    e.g., if the packet is an ARP reply, its CPU code assignment can be
    overridden with the specific CPU code CPSS_NET_ARP_REPLY_TO_ME_E
*/
UTF_TEST_CASE_MAC(prvTgfMcBridgingIpV6)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfMcBridgingIpV6ConfigSet();

    /* Generate traffic */
    prvTgfMcBridgingIpV6TrafficGenerate();

    /* Restore configuration */
    prvTgfMcBridgingIpV6ConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Dropping IPv6 MAC Multicast Filtering:
    Verify that packets with IP MC Address range are: 33-33-xx-xx-xx are dropped.
    Send different packets[IpV6 UC, IpV6 MC, NonIp UC, NonIp MC]
    and verify that multicast packets are dropped.
*/
UTF_TEST_CASE_MAC(prvTgfBrgDroppingIpV6MacMulticastFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfBrgDroppingIpV6MacMulticastFilteringConfigSet();

    /* Generate traffic */
    prvTgfBrgDroppingIpV6MacMulticastFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgDroppingIpV6MacMulticastFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Dropping Non- IPv4 MAC Multicast Filtering:
    Discard all non-Control-classified Ethernet packets with a MAC Multicast DA
    (but not the Broadcast MAC address) not corresponding to the IP Multicast range.
*/
UTF_TEST_CASE_MAC(prvTgfBrgDroppingNonIpV4MacMulticastFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigSet();

    /* Generate traffic */
    prvTgfBrgDroppingNonIpV4MacMulticastFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Per-InLIF port filtering functionality for reg\unreg MC IPv6 packet:
    configure VLAN, FDB entries;
    enable filtering with different commands;
    send IPv6 MC traffic and check expected traffic.
*/
/*
    Create Vlan 2 on all ports;
    Set default VLAN Id 2 on portIdx 0;
    Add MAC entry with macAddr 33:33:00:00:00:02 on portIdx 1, vlanId 2;
    Configure capturing;
    Generate traffic on portIdx 0 with vlanId 2, SA 00:88, DA 33:33:00:00:00:02;
    Check expected traffic;

    Enable filtering: DROP_HARD INLIF_UNREG_IP_MC, portIdx 0;
    Send unregistered IPv6 MC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 33:33:00:00:00:01;
    Check expected traffic;

    Send registered IPv6 MC packet -  generate traffic on portIdx 0;
     with vlanId 2, SA 00:88, DA 33:33:00:00:00:02;
    Check expected traffic;

    Enable filtering: trap2CPU INLIF_UNREG_IP_MC on portIdx 0;
    Send unregistered IPv6 MC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 33:33:00:00:00:01;
    Check expected traffic;

    Enable filtering: mirror2CPU INLIF_UNREG_IP_MC on portIdx 0;
    Send unregistered IPv6 MC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 33:33:00:00:00:01;
    Check expected traffic;

    Enable filtering: forward mode for INLIF_UNREG_IP_MC on portIdx 0;
    Send unregistered IPv6 MC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 33:33:00:00:00:01;
    Check expected traffic;

*/
UTF_TEST_CASE_MAC(prvTgfFdbPerInlifPortUnregIpv6McFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfFdbPerInlifPortUnregIpv6McFilteringConfigSet();

    /* Generate traffic */
    prvTgfFdbPerInlifPortUnregIpv6McFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbPerInlifPortUnregIpv6McFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Per-InLIF port filtering functionality for reg\unreg BC IPv4 packet:
    configure VLAN, FDB entries;
    enable filtering with different commands;
    send IPv4 BC traffic and check expected traffic.
*/
/*
    Create Vlan 2 on all ports;
    Set default VLAN Id 2 on portIdx 0;
    Add MAC entry with macAddr 01:55:00:00:00:02 on portIdx 1, vlanId 2;
    Configure capturing;
    Send unregistered nonIPv4 BC packet - generate traffic on portIdx 0
    with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

    Enable filtering: DROP_HARD INLIF_UNREG_IP, portIdx 0;
    Send unregistered nonIPv4 BC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

    Send registered IPv4 BC packet -  generate traffic on portIdx 0;
     with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

    Enable filtering: trap2CPU INLIF_UNREG_IP on portIdx 0;
    Send unregistered nonIPv4 BC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

    Enable filtering: mirror2CPU INLIF_UNREG_IP on portIdx 0;
    Send unregistered IPv4 BC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

    Enable filtering: forward mode for INLIF_UNREG_IP on portIdx 0;
    Send unregistered nonIPv4 BC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

*/
UTF_TEST_CASE_MAC(prvTgfFdbPerInlifPortUnregIpv4BcFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfFdbPerInlifPortUnregIpv4BcFilteringConfigSet();

    /* Generate traffic */
    prvTgfFdbPerInlifPortUnregIpv4BcFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbPerInlifPortUnregIpv4BcFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Security Breach Counters and dropped packets with invalid SA:
    configure VLAN, FDB entries;
    configure Security Breach drop mode;
    send traffic and check Security Breach Drop counters.
*/
/*
    Sets the Drop Mode for given Security Breach event - Soft Drop;
    Configure VLAN Security Breach Drop Counter to count security breach
     dropped packets based on their assigned vlanId 1;
    Configure capturing;
    Create FDB entry on portIdx 1, vlan 1,  macAddr 00:11,
     daCommand PRV_TGF_PACKET_CMD_CNTL_E - unconditionally trap to CPU;
    Configure drop counters - INVALID_MAC_SA;
    Send traffic on portIdx 0 with vlanId 1, SA 11:11:11:11:11:11, DA 00:11;
    Check expected traffic;
    Gets value of Security Breach Global Drop counter.

    Sets the Drop Mode for given Security Breach event - Hard Drop;
    Send traffic on portIdx 0 with vlanId 1, SA 11:11:11:11:11:11, DA 00:11;
    Check expected traffic;
*/
UTF_TEST_CASE_MAC(prvTgfFdbSecurityBreach)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* Set configuration */
    prvTgfFdbSecurityBreachConfigSet();

    /* Generate traffic */
    prvTgfFdbSecurityBreachTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbSecurityBreachConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Secure Auto Learning mode on packets with moved SA enabled:
    configure VLAN, FDB entry;
    configure Auto Learn Mode;
    configure Secure Auto Learn Mode;
    configure moved SA as security breach enable;
    send traffic and check counters.
*/
/*

*/
UTF_TEST_CASE_MAC(prvTgfFdbSecureAutoLearnMovedSa)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    /* not implemented in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);


    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->revision == 0)
    {
        PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA3_E);
    }
    else
    {
        PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA3_E);
    }

    /* Set configuration */
    prvTgfFdbSecureAutoLearnMovedSaConfigSet();

    /* Generate traffic */
    prvTgfFdbSecureAutoLearnMovedSaTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbSecureAutoLearnMovedSaConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Per-InLIF port filtering functionality for unknown UC DA:
    configure VLAN, FDB entries;
    enable unknown UC DA filtering with different commands;
    send unknown UC traffic and check expected traffic.
*/
/*
    Create Vlan 2 on all ports;
    Set port'sIdx 0 default VLAN Id 2;
    Add MAC entry with macAddr 00:02 on portIdx 1, vlanId 2;
    Configure capturing;
    Generate traffic on portIdx 0 with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;

    Enable filtering: drop hard unknown UC DA on portIdx 0;
    Send unknown UC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;

    Send known UC packet -  generate traffic on portIdx 0;
     with vlanId 2, SA 00:88, DA 00:02;
    Check expected traffic;

    Enable filtering: trap2CPU unknown UC DA on portIdx 0;
    Send unknown UC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;

    Enable filtering: mirror2CPU unknown UC DA on portIdx 0;
    Send unknown UC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;

    Enable filtering: forward mode for unknown UC DA on portIdx 0;
    Send unknown UC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;

*/
UTF_TEST_CASE_MAC(prvTgfFdbPerInlifPortUnknownUcFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfFdbPerInlifPortUnknownUcFilteringConfigSet();

    /* Generate traffic */
    prvTgfFdbPerInlifPortUnknownUcFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbPerInlifPortUnknownUcFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Per-Egress port filtering functionality for unknown UC DA:
    configure VLAN, FDB entries;
    enable egress filtering for bridged unknown UC packets;
    send unknown UC traffic and check expected traffic.
*/
/*
    Create Vlan 2 on all ports;
    Set port'sIdx 0 default VLAN Id 2;
    Add MAC entry with macAddr 00:02 on portIdx 1, vlanId 2;

    Generate traffic on portIdx 0 with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;

    Enable egress filtering for bridged unknown UC packets on ports
     with indexes 1, 2, 3;
    Send unknown UC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;

    Send known UC packet -  generate traffic on portIdx 0;
     with vlanId 2, SA 00:88, DA 00:02;
    Check expected traffic;

    Disable egress filtering for bridged unknown UC packets on all ports;
    Send unknown UC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 00:01;
    Check expected traffic;
*/
UTF_TEST_CASE_MAC(prvTgfFdbPerEgressPortUnknownUcFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* Set configuration */
    prvTgfFdbPerEgressPortUnknownUcFilteringConfigSet();

    /* Generate traffic */
    prvTgfFdbPerEgressPortUnknownUcFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbPerEgressPortUnknownUcFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Per-Egress port filtering functionality for unreg IPv4 MC:
    configure VLAN, FDB entries;
    enable egress filtering for bridged unregistered MC packets;
    send IPv4 MC traffic and check expected traffic.
*/
/*
    Create Vlan 2 on all ports;
    Set port'sIdx 0 default VLAN Id 2;
    Add MAC entry with macAddr 01:00:5E:00:00:02 on portIdx 1, vlanId 2;

    Generate traffic - unregistered MC IPv4 packet on portIdx 0 with vlanId 2,
     SA 00:88, DA 01:00:5E:00:00:01;
    Check expected traffic;

    Enable egress filtering for bridged unregistered multicast packets
     on ports with indexes 1, 2, 3;

    Send unregistered IPv4 MC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 01:00:5E:00:00:01;
    Check expected traffic;

    Send known IPv4 MC packet -  generate traffic on portIdx 0;
     with vlanId 2, SA 00:88, DA 01:00:5E:00:00:02;
    Check expected traffic;

    Disable egress filtering for bridged unregistered multicast packets
     on ports with indexes 1, 2, 3;

    Send unregistered IPv4 MC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA 01:00:5E:00:00:01;
    Check expected traffic;
*/
UTF_TEST_CASE_MAC(prvTgfFdbPerEgressPortUnregIpv4McFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    /* test not supported in multi port group FDB lookup system */
    PRV_TGF_MULTI_PORT_GROUP_FDB_LOOKUP_SYSTEM_LIMITATION_REDIRECT_TO_VID_INTERFACE_FROM_FDB_CHECK_MAC(
            prvTgfFdbPerEgressPortUnregIpv4McFiltering);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigSet();

    /* Generate traffic */
    prvTgfFdbPerEgressPortUnregIpv4McFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbPerEgressPortUnregIpv4McFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify Per-Egress port filtering functionality for unreg IPv6 BC:
    configure VLAN, FDB entries;
    enable egress filtering for bridged unregistered BC packets;
    send IPv6 BC traffic and check expected traffic.
*/
/*
    Test - Verify Per-Egress port filtering for unregistered IPv6
           broadcast packets.

    Create Vlan 2 on all ports;
    Set port'sIdx 0 default VLAN Id 2;
    Add MAC entry with macAddr 00:00:00:00:00:02 on portIdx 1, vlanId 2;

    Generate traffic - unregistered BC IPv6 packet on portIdx 0 with vlanId 2,
     SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

    Enable egress filtering for bridged unregistered multicast packets
     on ports with indexes 1, 2, 3;

    Send unregistered IPv6 BC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;

    Send known packet -  generate traffic on portIdx 0 with vlanId 2,
     SA 00:88, DA 00:02;
    Check expected traffic;

    Disable egress filtering for bridged unregistered multicast packets
     on ports with indexes 1, 2, 3;

    Send unregistered IPv6 BC packet -  generate traffic on portIdx 0
     with vlanId 2, SA 00:88, DA FF:FF:FF:FF:FF:FF;
    Check expected traffic;
*/
UTF_TEST_CASE_MAC(prvTgfFdbPerEgressPortUnregIpv6BcFiltering)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* test not supported in multi port group FDB lookup system */
    PRV_TGF_MULTI_PORT_GROUP_FDB_LOOKUP_SYSTEM_LIMITATION_REDIRECT_TO_VID_INTERFACE_FROM_FDB_CHECK_MAC(
            prvTgfFdbPerEgressPortUnregIpv6BcFiltering);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFdbPerEgressPortUnregIpv6BcFilteringConfigSet();

    /* Generate traffic */
    prvTgfFdbPerEgressPortUnregIpv6BcFilteringTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbPerEgressPortUnregIpv6BcFilteringConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify receiving packets with same UC location as orig src location:
    configure VLAN, FDB entries;
    sets local switching through the ingress interface for known UC packets;
    send IPv4 UC traffic and check expected traffic.
*/
/*
    Create Vlan 2 on all ports;
    Set port'sIdx 0 default VLAN Id 2;

    Sets local switching through the ingress interface for known UC packets;

    Send IPv4 UC packets from portIdx 0 - generate traffic with
     DA 00:02, SA 00:00, vlanId 2;
    Check expected traffic;

    Add MAC entry with macAddr 00:02 on portIdx 0, vlanId 2;

    Send IPv4 UC packets from portIdx 0 - generate traffic with
     DA 00:02, SA 00:00, vlanId 2;
    Check expected traffic;

    Disble local switching;

    Send IPv4 UC packets from portIdx 0 - generate traffic with
     DA 00:02, SA 00:00, vlanId 2;
    Check expected traffic;
*/
UTF_TEST_CASE_MAC(prvTgfFdbLocalUcSwitching)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E | UTF_PUMA_E);

    /* Set configuration */
    prvTgfFdbLocalUcSwitchingConfigSet();

    /* Generate traffic */
    prvTgfFdbLocalUcSwitchingTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbLocalUcSwitchingConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify receiving packets with same MC location as orig src location:
    configure VLAN, FDB entries;
    sets local switching through the ingress interface for known MC packets;
    send IPv4 MC traffic and check expected traffic.
*/
/*
    Create trunk with trunkId 1 and add portIdxs members [0, 3];

    Create Vlan 2 on all ports;

    Enable Mc local switching;

    Send IPv4 MC packets from portIdx 0 - generate traffic with
     DA 01:00:5e:00:00:02, SA 00:00, vlanId 2;
    Check expected traffic;

    Create McGroup on vidx 1 with port member 0;

    Create Fdb macEntry with MAC 01:00:5e:00:00:02 port 0, vlanId 2, vidx 1;

    Resend previous traffic;
    Check expected traffic;

    Disble Mc local switching;

    Resend previous traffic;
    Check expected traffic;
*/
UTF_TEST_CASE_MAC(prvTgfFdbLocalMcSwitching)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_PUMA_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);


    /* Set configuration */
    prvTgfFdbLocalMcSwitchingConfigSet();

    /* Generate traffic */
    prvTgfFdbLocalMcSwitchingTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbLocalMcSwitchingConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    calculate FDB entry index by means of calling HASH function
    configure FDB uc ipv4 route entry in figured index
    send IPv4 UC packet and check expected traffic
    check IP counters values.
*/
UTF_TEST_CASE_MAC(prvTgfFdbBasicIpv4UcRoutingVlanModeIpCounters)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbBasicIpv4UcRoutingRouteConfigurationSet(GT_TRUE);

    /* Configure VLAN mode IP counters and generate traffic */
    prvTgfFdbBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_TRUE, GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbBasicIpv4UcRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    calculate FDB entry index by means of calling HASH function
    configure FDB uc ipv4 route entry in figured index
    configure IP counters in VLAN mode
    send IPv4 UC packet and check expected traffic
    check IP counters values.
*/
UTF_TEST_CASE_MAC(prvTgfFdbBasicIpv4UcRoutingByIndex)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbBasicIpv4UcRoutingRouteConfigurationSet(GT_TRUE);

    /* Generate traffic */
    prvTgfFdbBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_TRUE, GT_FALSE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbBasicIpv4UcRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    configure FDB uc ipv4 route entry
    send IPv4 UC packet and check expected traffic.
*/
UTF_TEST_CASE_MAC(prvTgfFdbBasicIpv4UcRoutingByMsg)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);
    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbBasicIpv4UcRoutingRouteConfigurationSet(GT_FALSE);

    /* Generate traffic */
    prvTgfFdbBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], GT_TRUE, GT_FALSE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbBasicIpv4UcRoutingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    congiure FDB routing
    configure ECMP entry
    send traffic and check that next hop info is taken from ECMP entry
    restore configuration
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcPointerRouteByIndex)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);
    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbIpv4UcPointerRouteConfigurationSet(GT_TRUE);

    /* Set ECMP Route configuration */
    prvTgfFdbIpv4UcPointerRouteEcmpRoutingConfigurationSet(GT_FALSE,GT_FALSE);


    prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate(GT_FALSE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4UcPointerRouteConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    congiure FDB routing
    configure ECMP entry
    send traffic and check that next hop info is taken from ECMP entry
    restore configuration
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcPointerRouteByMsg)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);
    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbIpv4UcPointerRouteConfigurationSet(GT_FALSE);

    /* Set ECMP Route configuration */
    prvTgfFdbIpv4UcPointerRouteEcmpRoutingConfigurationSet(GT_FALSE,GT_FALSE);

    prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate(GT_FALSE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4UcPointerRouteConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    calculate FDB entry index by means of calling HASH function
    configure FDB uc ipv4 route entry in figured index
    configure IP counters in VLAN mode
    send IPv4 UC packet and check expected traffic
    check IP counters values.
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcRoutingLookupMask)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by SIP6 devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);
    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbIpv4UcRoutingLookupMaskRouteConfigurationSet(GT_TRUE);

    /* Test FDB IPv4 Lookup Mask */
    prvTgfFdbIpv4UcRoutingLookupMaskTest();

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4UcRoutingLookupMaskConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    calculate FDB entry index by means of calling HASH function
    configure FDB uc ipv4 route entry in figured index
    send IPv4 UC packet and check expected traffic.
*/
UTF_TEST_CASE_MAC(prvTgfFdbBasicIpv4UcRoutingFillFdbByIndex)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E);

    /* the test is long due to many packets */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* too many packets for GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    prvTgfFdbBasicIpv4UcRoutingFillFdb(GT_TRUE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    configure FDB uc ipv4 route entry
    send IPv4 UC packet and check expected traffic.
*/
UTF_TEST_CASE_MAC(prvTgfFdbBasicIpv4UcRoutingFillFdbByMessage)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E);


    /* the test is long due to many packets */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    /* too many packets for GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    prvTgfFdbBasicIpv4UcRoutingFillFdb(GT_FALSE);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure FDB routing trigering conditions
    configure FDB uc ipv4 route entry
    configure FDB Routing Delete=GT_FALSE
    send IPv4 UC packet
    check that flush FDB do not delete the UC entries and that traffic is as expected
    configure FDB Routing Delete=GT_TRUE
    send IPv4 UC packet
    check that flush FDB do delete the UC entries and that there is no traffic
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcRoutingDeleteEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbIpv4UcRoutingDeleteEnableConfigurationSet(GT_FALSE);

    /* Generate traffic - expect traffic on ports 2.3 */
    prvTgfFdbIpv4UcRoutingDeleteEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],GT_TRUE);

    /* enable Delete, flush FDB expect no trafic */
    prvTgfFdbIpv4UcRoutingDeleteEnableSet(GT_TRUE);

    /* Generate traffic -  expect no trafic */
    prvTgfFdbIpv4UcRoutingDeleteEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],GT_FALSE);

    /* add again the UC entry */
    prvTgfFdbIpv4UcRoutingDeleteEnableConfigurationSet(GT_FALSE);

    /* disable Delete, flush FDB expect traffic */
    prvTgfFdbIpv4UcRoutingDeleteEnableSet(GT_FALSE);

    /* Generate traffic -  expect trafic */
    prvTgfFdbIpv4UcRoutingDeleteEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4UcRoutingDeleteEnableConfigurationRestore();
}

/* look for 2 port indexes that represent ports in the same FDB instance */
static  void getPortsInTheSameFdbInstance(
    OUT GT_U32  *portIndex1Ptr,
    OUT GT_U32  *portIndex2Ptr
)
{
    GT_STATUS       rc;
    GT_U8   devNum = prvTgfDevNum;
    GT_U32  pipeId1,pipeId2;
    GT_U32  tileId1,tileId2;
    GT_U32  ii,jj;
    GT_BOOL firstFail = GT_TRUE;
    GT_U8   orig_prvTgfPortsNum = prvTgfPortsNum;

    *portIndex1Ptr = 0;/* one port must be at index 0..1 */
    *portIndex2Ptr = 3;/* one port must be at index 2..3 */

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles < 2)
    {
        return;
    }

retry_lbl:
    for(ii = 0; ii < (GT_U32)(prvTgfPortsNum-1); ii++)
    {
        rc = prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert(
            devNum,prvTgfPortsArray[ii],&pipeId1);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
            return;
        }

        tileId1 = pipeId1 / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

        for(jj = ii+1; jj < prvTgfPortsNum; jj++)
        {
            rc = prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert(
                devNum,prvTgfPortsArray[jj],&pipeId2);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
                return;
            }

            tileId2 = pipeId2 / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

            if(tileId1 == tileId2)
            {

                if(firstFail == GT_FALSE)
                {
                    /* we need to replace 2 ports in the array 0..3 */
                    if(ii < orig_prvTgfPortsNum)
                    {
                        /* one port must be at index 0..1 */
                        /* one port must be at index 2..3 */

                        prvTgfPortsArray[(ii+2)%orig_prvTgfPortsNum] = prvTgfPortsArray[jj];
                        jj = (ii+2)%orig_prvTgfPortsNum;
                    }
                    else /* both ii,jj >=4 */
                    {
                        prvTgfPortsArray[(ii+0)%orig_prvTgfPortsNum] = prvTgfPortsArray[ii];/* one port must be at index 0..1 */
                        prvTgfPortsArray[(ii+2)%orig_prvTgfPortsNum] = prvTgfPortsArray[jj];/* one port must be at index 2..3 */
                        ii = (ii+0)%orig_prvTgfPortsNum;
                        jj = (ii+2)%orig_prvTgfPortsNum;
                    }

                    prvTgfPortsNum = orig_prvTgfPortsNum;
                }


                /* found 2 ports in the same tile ... same FDB unit */
                *portIndex1Ptr = ii;
                *portIndex2Ptr = jj;

                /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
                TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_INSTANCE_PER_TILE_MAC(prvTgfPortsArray[ii]);

                return;
            }
        }
    }

    /* not found ... need wider range */
    if(firstFail == GT_TRUE)
    {
        firstFail = GT_FALSE;

        prvTgfPortsNum = 12;

        goto retry_lbl;
    }

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,GT_NOT_FOUND);

    return;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure FDB routing trigering conditions
    configure FDB uc ipv4 route entry with age=GT_FALSE
    configure FDB Routing Refresh=GT_TRUE
    send IPv4 UC packet
    check that traffic is as expected and the age was changed to GT_TRUE
    configure FDB Routing Refresh=GT_FALSE
    send IPv4 UC packet
    check that traffic is as expected and the age is GT_FALSE
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcRoutingRefreshEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;
    GT_U32      port1Index,port2Index;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* get 2 ports in the same FDB instance */
    getPortsInTheSameFdbInstance(&port1Index,&port2Index);

    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration by index */
    prvTgfFdbIpv4UcRoutingRefreshEnableConfigurationSet(GT_TRUE,port2Index);

    /* Enable Refresh */
    prvTgfFdbIpv4UcRoutingRefreshEnableSet(GT_TRUE);

    /* Generate traffic - expect traffic on ports 3 and the age was changed to GT_TRUE */
    prvTgfFdbIpv4UcRoutingRefreshEnableTrafficGenerate(prvTgfPortsArray[port1Index], prvTgfPortsArray[port2Index],GT_TRUE);

    /* Expect valid=true, skip=false, age=true */
    prvTgfFdbIpv4UcRoutingRefreshCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_TRUE);

    /* Disable Refresh */
    prvTgfFdbIpv4UcRoutingRefreshEnableSet(GT_FALSE);

    /* Add again the entry with age=GT_FALSE */
    prvTgfFdbIpv4UcRoutingRefreshEnableConfigurationSet(GT_TRUE,port2Index);

    /* Generate traffic - expect traffic on ports 3 and the age is GT_FALSE */
    prvTgfFdbIpv4UcRoutingRefreshEnableTrafficGenerate(prvTgfPortsArray[port1Index], prvTgfPortsArray[port2Index],GT_TRUE);

    /* Expect valid=true, skip=false, age=false */
    prvTgfFdbIpv4UcRoutingRefreshCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4UcRoutingRefreshEnableConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    Transplant FDB Ipv4 Route - Basic

    Test Goals:
      Verify that ipv4 entries are not transplanted by default.
      Verify that ipv4 entries are transplanted after enable transplanting IP UC entries.

    set FDB Ipv4 Routing entries :
    ipv4 1.1.1.3 in vlan 6 on port 2 , static

    send traffic from port 2, ipv4=1.1.1.3 check packet arrive to port 2

    set transplant parameters : old interface = port 2 , new interface = port 3
    trigger the mac action : transplant (do transplant)
    trigger the FDB_Routing trasplant (do transplant)
    ===
    send traffic from port 0, ipv4=1.1.1.3 check packet arrive to port 2
    ===
    enable the transplant of static entries
    ===
    send traffic from port 0, ipv4=1.1.1.3 check packet arrive to port 3
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcRoutingTransplantEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    GM_NOT_SUPPORT_THIS_TEST_MAC

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration by index */
    prvTgfFdbIpv4UcRoutingTransplantEnableConfigurationSet(GT_TRUE);

    /* Generate traffic - expect traffic on ports 2 */
    prvTgfFdbIpv4UcRoutingTransplantEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[2],GT_TRUE);

    /* Enable and trigger FDB Transplant */
    prvTgfFdbIpv4UcRoutingTransplantEnableSet(prvTgfPortsArray[2],prvTgfPortsArray[3],GT_TRUE);

    /* Generate traffic - expect traffic on ports 3 */
    prvTgfFdbIpv4UcRoutingTransplantEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],GT_TRUE);

    /* Add again the entry with port 2 */
    prvTgfFdbIpv4UcRoutingTransplantEnableConfigurationSet(GT_TRUE);

    /* Disable and trigger FDB Transplant */
    prvTgfFdbIpv4UcRoutingTransplantEnableSet(prvTgfPortsArray[2],prvTgfPortsArray[3],GT_FALSE);

    /* Generate traffic - expect traffic on ports 2 */
    prvTgfFdbIpv4UcRoutingTransplantEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[2],GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4UcRoutingTransplantEnableConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    Triggered Aging with/without removal FDB Ipv4 Route

    Test Goals:
    Verify that addresses are aged after perform triggered aging twice.

    set FDB Ipv4 Routing entries :
    ipv4 1.1.1.3 in vlan 6 on port 2 , static
    ===
    Enable FDB Routing aging
    ===
    (1)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Enaled
    ===
    check FDB addresse exists (no aging yet)
    ===
    (2)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Enaled
    =====
    check FDB content- entry was deleted in case of aged_without_removal + AA_TA_Msg_To_Cpu
    ===
    add agin FDB Ipv4 Routing entries :
    ipv4 1.1.1.3 in vlan 6 on port 2 , static
    =====
    (1)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Disable
    ===
    check FDB addresse exists (no aging yet)
    ===
    (2)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Disable
    =====
    check FDB content- entry was NOT deleted in case of aged_without_removal
    ===
    add agin FDB Ipv4 Routing entries :
    ipv4 1.1.1.3 in vlan 6 on port 2 , static
    ===
    Disable FDB Routing aging + AA_TA_Msg_To_Cpu enabled
    ====
    (1)trigger the FDB_Routing Aging (do aging)
    ===
    check FDB all addresses exists (no aging yet)
    ===
    (2)trigger the FDB_Routing Aging (do aging)
    =====
    check FDB content- entry is not aged since FDB Routing aging is "disabled"
    =====

*/

UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcRoutingAgingEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* for GM FDB_Routing Aging mechanism is not supported */
    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        SKIP_TEST_MAC
    }

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    /* Set Base configuration */
    prvTgfFdbBasicIpv4UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationSet();

    /* Set Route configuration by index */
    prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* AGE WITHOUT REMOVAL */

    /* Enable and trigger FDB Aging without removal, AA and TA Msg To Cpu Enaled */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false (first trigger changed it from true to false) */
    prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE);

    /*trigger the FDB_Routing Aging (do aging) without removal - second trigger send an AA_TA msg to CPU */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_TRUE);

    /* Expect valid=false, skip=false, age=false - entry was deleted - aging without removal
       but the entry should be deleted due to msg to CPU  */
    if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum, PRV_CPSS_DXCH_BOBCAT2_FDB_ROUTE_UC_DELETE_BY_MESSAGE_WA_E))
    {
        prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_FALSE, GT_FALSE, GT_FALSE);
    }
    else
    {
        prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_FALSE, GT_TRUE, GT_FALSE);
    }

    /* add againg the entry */
    prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* Enable and trigger FDB Aging without removal, AA and TA Msg To Cpu Disabled */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false (first trigger changed it from true to false) */
    prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE);

    /*trigger the FDB_Routing Aging (do aging) without removal - second trigger do not send an AA_TA msg to CPU */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false - entry was not deleted - aging without removal, no msg to CPU */
    prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE);

    /* add againg the entry */
    prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* AGE WITH REMOVAL */

    /* Enable and trigger FDB Aging with removal, AA and TA Msg To Cpu Disabled */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_TRUE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false (first trigger changed it from true to false) */
    prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE);

    /*trigger the FDB_Routing Aging (do aging) with removal - second trigger do not send
      an AA_TA msg to CPU but DO NOT delete the entry -
      All UC route entries are treated as age_without_removal even if action is age_with_removal */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_TRUE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false - entry was NOT deleted -
       aging with removal treated as age_without_removal, no msg to CPU */
    prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE);

    /* add againg the entry */
    prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* DISABLE AGE */

    /* Disable and trigger FDB Aging */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_FALSE,GT_FALSE,GT_TRUE);

    /* Expect valid=true, skip=false, age=true - no aging on the entry */
    prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_TRUE);

    /*trigger the FDB_Routing Aging (do aging) */
    prvTgfFdbIpv4UcRoutingAgingEnableSet(GT_FALSE,GT_FALSE,GT_TRUE);

    /* Expect valid=true, skip=false, age=true - no aging on the entry  */
    prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv4UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv4UcRoutingAgingEnableConfigurationRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    calculate FDB entry index by means of calling HASH function
    configure FDB uc ipv6 route entry in figured index
    send IPv6 UC packet and check expected traffic.
*/
UTF_TEST_CASE_MAC(prvTgfFdbBasicIpv6UcRouting)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Base configuration */
    prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbBasicIpv6UcRoutingRouteConfigurationSet();

    /* Generate traffic */
    prvTgfFdbBasicIpv6UcRoutingTrafficGenerate(GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
     prvTgfFdbBasicIpv6UcRoutingConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure CRC Multi HASH mode
    configure FDB routing trigering conditions
    calculate FDB entry index by means of calling HASH function
    configure FDB uc ipv6 route entry in figured index
    send IPv6 UC packet and check expected traffic.
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv6UcRoutingLookupMask)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    /* Supported by SIP6 devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_CPSS_PP_ALL_SIP6_CNS);
    /* Set Base configuration */
    prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbIpv6UcRoutingLookupMaskRouteConfigurationSet();

    /* Test FDB IPv6 Lookup Mask */
    prvTgfFdbIpv6UcRoutingLookupMaskTest();

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
     prvTgfFdbIpv6UcRoutingLookupMaskConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure FDB routing trigering conditions
    configure FDB uc ipv6 route entry
    configure FDB Routing Delete=GT_FALSE
    send IPv6 UC packet
    check that flush FDB do not delete the UC entries and that traffic is as expected
    configure FDB Routing Delete=GT_TRUE
    send IPv6 UC packet
    check that flush FDB do delete the UC entries and that there is no traffic
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv6UcRoutingDeleteEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* Set Base configuration */
    prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet();

    /* Set Route configuration */
    prvTgfFdbIpv6UcRoutingDeleteEnableConfigurationSet();

    /* Generate traffic - expect traffic on ports 2.3 */
    prvTgfFdbIpv6UcRoutingDeleteEnableTrafficGenerate(GT_TRUE);

    /* enable Delete, flush FDB expect no trafic */
    prvTgfFdbIpv6UcRoutingDeleteEnableSet(GT_TRUE);

    /* Generate traffic -  expect no trafic */
    prvTgfFdbIpv6UcRoutingDeleteEnableTrafficGenerate(GT_FALSE);

    /* add again the UC entry */
    prvTgfFdbIpv6UcRoutingDeleteEnableConfigurationSet();

    /* disable Delete, flush FDB expect traffic */
    prvTgfFdbIpv6UcRoutingDeleteEnableSet(GT_FALSE);

    /* Generate traffic -  expect trafic */
    prvTgfFdbIpv6UcRoutingDeleteEnableTrafficGenerate(GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv6UcRoutingDeleteEnableConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    configure VLANs
    configure FDB routing trigering conditions
    configure FDB uc ipv6 route entry with age=GT_FALSE
    configure FDB Routing Refresh=GT_TRUE
    send IPv6 UC packet
    check that traffic is as expected and the age was changed to GT_TRUE
    configure FDB Routing Refresh=GT_FALSE
    send IPv6 UC packet
    check that traffic is as expected and the age is GT_FALSE
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv6UcRoutingRefreshEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set Base configuration */
    prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet();

    /* Set Route configuration by index */
    prvTgfFdbIpv6UcRoutingRefreshEnableConfigurationSet();

    /* Enable Refresh */
    prvTgfFdbIpv6UcRoutingRefreshEnableSet(GT_TRUE);

    /* Generate traffic - expect traffic on ports 3 and the age was changed to GT_TRUE */
    prvTgfFdbIpv6UcRoutingRefreshEnableTrafficGenerate(GT_TRUE);

    /* Expect valid=true, skip=false, age=true - refresh is only done to the address and not to the data */
    prvTgfFdbIpv6UcRoutingRefreshCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_TRUE,GT_TRUE,GT_FALSE,GT_FALSE);

    /* Disable Refresh */
    prvTgfFdbIpv6UcRoutingRefreshEnableSet(GT_FALSE);

    /* Add again the entry with age=GT_FALSE */
    prvTgfFdbIpv6UcRoutingRefreshEnableConfigurationSet();

    /* Generate traffic - expect traffic on ports 3 and the age is GT_FALSE */
    prvTgfFdbIpv6UcRoutingRefreshEnableTrafficGenerate(GT_TRUE);

    /* Expect valid=true, skip=false, age=false refresh is only done to the address and not to the data */
    prvTgfFdbIpv6UcRoutingRefreshCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE,GT_TRUE,GT_FALSE,GT_FALSE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv6UcRoutingRefreshEnableConfigurationRestore();
}

#if 0
/* ipv6 transplant is not supported */

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    Transplant FDB Ipv6 Route - Basic

    Test Goals:
      Verify that ipv6 entries are not transplanted by default.
      Verify that ipv6 entries are transplanted after enable transplanting IP UC entries.

    set FDB Ipv6 Routing entries :
    ipv6 1.1.1.3 in vlan 6 on port 2 , static

    send traffic from port 2, ipv6=1122::ccdd:eeff check packet arrive to port 2

    set transplant parameters : old interface = port 2 , new interface = port 3
    trigger the mac action : transplant (do transplant)
    trigger the FDB_Routing trasplant (do transplant)
    ===
    send traffic from port 0, ipv6=1122::ccdd:eeff check packet arrive to port 2
    ===
    enable the transplant of static entries
    ===
    send traffic from port 0, ipv6=1122::ccdd:eeff check packet arrive to port 3
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv6UcRoutingTransplantEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Set Base configuration */
    prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet();

    /* Set Route configuration by index */
    prvTgfFdbIpv6UcRoutingTransplantEnableConfigurationSet();

    /* Generate traffic - expect traffic on ports 2 */
    prvTgfFdbIpv6UcRoutingTransplantEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[2],GT_TRUE);

    /* Enable and trigger FDB Transplant */
    prvTgfFdbIpv6UcRoutingTransplantEnableSet(prvTgfPortsArray[2],prvTgfPortsArray[3],GT_TRUE);

    /* Generate traffic - expect traffic on ports 3 */
    prvTgfFdbIpv6UcRoutingTransplantEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],GT_TRUE);

    /* Add again the entry with port 2 */
    prvTgfFdbIpv6UcRoutingTransplantEnableConfigurationSet();

    /* Disable and trigger FDB Transplant */
    prvTgfFdbIpv6UcRoutingTransplantEnableSet(prvTgfPortsArray[2],prvTgfPortsArray[3],GT_FALSE);

    /* Generate traffic - expect traffic on ports 2 */
    prvTgfFdbIpv6UcRoutingTransplantEnableTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[2],GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv6UcRoutingTransplantEnableConfigurationRestore();

}
#endif

/*----------------------------------------------------------------------------*/
/* AUTODOC:
    Triggered Aging with/without removal FDB Ipv6 Route

    Test Goals:
    Verify that addresses are aged after perform triggered aging twice.

    set FDB Ipv6 Routing entries
    ===
    Enable FDB Routing aging
    ===
    (1)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Disable
    ===
    check FDB addresse exists (no aging yet)
    ===
    (2)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Enaled
    =====
    check FDB content- entry was deleted in case of aged_without_removal + AA_TA_Msg_To_Cpu -- added support in appDemo
    ===
    add again FDB Ipv6 Routing entries
    =====
    (1)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Disable
    ===
    check FDB addresse exists (no aging yet)
    ===
    (2)trigger the FDB_Routing Aging (do aging) AA and TA Msg To Cpu Disable
    =====
    check FDB content- entry was NOT deleted in case of aged_without_removal
    ===
    add again FDB Ipv6 Routing entries :
    ===
    Disable FDB Routing aging + AA_TA_Msg_To_Cpu enabled
    ====
    (1)trigger the FDB_Routing Aging (do aging)
    ===
    check FDB all addresses exists (no aging yet)
    ===
    (2)trigger the FDB_Routing Aging (do aging)
    =====
    check FDB content- entry is not aged since FDB Routing aging is "disabled"
    =====

*/

UTF_TEST_CASE_MAC(prvTgfFdbIpv6UcRoutingAgingEnable)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* for GM FDB_Routing Aging mechanism is not supported */
    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        SKIP_TEST_MAC
    }

    /* Set Base configuration */
    prvTgfFdbBasicIpv6UcRoutingBaseConfigurationSet();

    /* Set Generic Route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationSet();

    /* Set Route configuration by index */
    prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* AGE WITHOUT REMOVAL */

    /* Enable and trigger FDB Aging without removal, AA and TA Msg To Cpu Enaled */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_FALSE);

    /* Address: Expect valid=true, skip=false, age=false (first trigger changed it from true to false)
       Data - not examined for aging: Expect valid=true, skip=false, age=true
       (first trigger do not changed it from true to false)   */
    prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE,GT_TRUE,GT_FALSE,GT_TRUE);

    /*trigger the FDB_Routing Aging (do aging) without removal - second trigger send an AA_TA msg to CPU */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_TRUE);

    /* Address:  Expect valid=false, skip=false, age=false - entry was deleted - aging without removal
       but the entry should be deleted due to msg to CPU
       Data - not examined for aging:  Expect valid=false, skip=false, age=false - entry was deleted - aging without removal
       but the entry should be deleted due to msg to CPU - added support in appDemo to delete the data related to a given address */
    if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum, PRV_CPSS_DXCH_BOBCAT2_FDB_ROUTE_UC_DELETE_BY_MESSAGE_WA_E))
    {
        prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE);
    }
    else
    {
        prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_TRUE);
    }

    /* add againg the entry */
    prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* Enable and trigger FDB Aging without removal, AA and TA Msg To Cpu Disabled */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false (first trigger changed it from true to false) */
    prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE,GT_TRUE,GT_FALSE,GT_TRUE);

    /*trigger the FDB_Routing Aging (do aging) without removal - second trigger do not send an AA_TA msg to CPU */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_FALSE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false - entry was not deleted - aging without removal, no msg to CPU */
    prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE,GT_TRUE,GT_FALSE,GT_TRUE);

    /* add againg the entry */
    prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* AGE WITH REMOVAL */

    /* Enable and trigger FDB Aging with removal, AA and TA Msg To Cpu Disabled */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_TRUE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false (first trigger changed it from true to false) */
    prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE,GT_TRUE,GT_FALSE,GT_TRUE);

    /*trigger the FDB_Routing Aging (do aging) with removal - second trigger do not send
      an AA_TA msg to CPU but DO NOT delete the entry -
      All UC route entries are treated as age_without_removal even if action is age_with_removal */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_TRUE,GT_TRUE,GT_FALSE);

    /* Expect valid=true, skip=false, age=false - entry was NOT deleted -
       aging with removal treated as age_without_removal, no msg to CPU */
    prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_FALSE,GT_TRUE,GT_FALSE,GT_TRUE);

    /* add againg the entry */
    prvTgfFdbIpv6UcRoutingAgingEnableConfigurationSet(GT_TRUE);

    /* DISABLE AGE */

    /* Disable and trigger FDB Aging */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_FALSE,GT_FALSE,GT_TRUE);

    /* Expect valid=true, skip=false, age=true - no aging on the entry */
    prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_TRUE,GT_TRUE,GT_FALSE,GT_TRUE);

    /*trigger the FDB_Routing Aging (do aging) */
    prvTgfFdbIpv6UcRoutingAgingEnableSet(GT_FALSE,GT_FALSE,GT_TRUE);

    /* Expect valid=true, skip=false, age=true - no aging on the entry  */
    prvTgfFdbIpv6UcRoutingAgingCheckValidSkipAgeValues(GT_TRUE,GT_FALSE,GT_TRUE,GT_TRUE,GT_FALSE,GT_TRUE);

    /* Restore generic route configuration */
    prvTgfFdbBasicIpv6UcGenericRoutingConfigurationRestore();

    /* Restore configuration */
    prvTgfFdbIpv6UcRoutingAgingEnableConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify that there are no discarded packets during update of existing
    VLAN table entry via direct access mode.

    Run full-wire speed traffic test on 10G ports (4 ports, 4 VLAN entries)
    Write dummy VLAN entry with vlanId = 105
    Write all existing VLANs 5,6,7,8 without changes
    Check eggress counter - number of discarded packets
*/
UTF_TEST_CASE_MAC(prvTgfVlanTableWriteWorkaround)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

/*
    1. Generate traffic
    2. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_PUMA_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* Generate traffic */
    prvTgfBrgVlanTableWriteWorkaroundTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanTableWriteWorkaroundConfigurationRestore();

#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify that there are no discarded packets during update of existing
           STG table entry via direct access mode.

    Run full-wire speed traffic test on 10G ports
    Write dummy STG entry with groupId = 1 and block command for all ports
    Write default STG entry with groupId = 0 without changes
    Check eggress counter - number of discarded packets
*/
UTF_TEST_CASE_MAC(prvTgfStgTableWriteWorkaround)
{
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

/*
    1. Generate traffic
    2. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_PUMA_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* Generate traffic */
    prvTgfBrgStgTableWriteWorkaroundTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgStgTableWriteWorkaroundConfigurationRestore();

#endif
}

#ifdef ASIC_SIMULATION
/* by default test not run in WM because we have other non-stable tests under 'stress' */
static GT_BOOL wm_prvTgfFdbTableReadUnderWireSpeed = GT_FALSE;
void wm_prvTgfFdbTableReadUnderWireSpeedSet(IN GT_BOOL allowWm)
{
    wm_prvTgfFdbTableReadUnderWireSpeed = allowWm;
}
#endif
/*----------------------------------------------------------------------------*/
/* AUTODOC: Verify that there are no discarded packets during read access by the
    CPU to the FDB.

    Run full-wire speed traffic test on 10G ports (4 ports, 4 VLAN entries)
    static mac addresses that relate to macSA,DA
    Check bridge drop counters - must be 0
*/
UTF_TEST_CASE_MAC(prvTgfFdbTableReadUnderWireSpeed)
{
#ifndef ASIC_SIMULATION
    GT_BOOL supportTest = GT_TRUE;
#else
    GT_BOOL supportTest = wm_prvTgfFdbTableReadUnderWireSpeed;
#endif
    if(GT_TRUE == prvTgfBrgLoopbackCt10And40Check())
    {
        return;
    }

/*
    1. Generate traffic
    2. Restore configuration
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_PUMA_E);

    /* GM simulation not support this --> skip */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if(supportTest == GT_FALSE)
    {
        SKIP_TEST_MAC;
    }
    /* Generate traffic */
    prvTgfFdbTableReadUnderWireSpeedTrafficGenerate();

    /* Restore configuration */
    prvTgfFdbTableReadUnderWireSpeedConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test nested core to customer:
    Set default vlan entry 3 with portIdx members 0,2;
    Configure tagged portIdx 0 as core port;
    Configure untagged portIdx 2 as customer port;
    Send double tagged traffic with S-VLAN and
    C-VLAN tags to core port and expect tagging
    traffic with C-VLAN tag only on customer port.
*/
UTF_TEST_CASE_MAC(prvTgfBrgNestedCoreToCustomer)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* Set configuration */
    prvTgfBrgNestedCoreToCustomerConfigSet();

    /* Generate traffic */
    prvTgfBrgNestedCoreToCustomerTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgNestedCoreToCustomerConfigRestore();
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test prvTgfBrgNestedCoreToCustomer when 'capture' using mirror mode
    'src based' and not 'hop by hop' (default)
*/
UTF_TEST_CASE_MAC(prvTgfBrgNestedCoreToCustomer_captureUseSrcBasedMirror)
{
    PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT oldMode;
    /* save mirror old mode */
    prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum,&oldMode);
    /* set new mirror mode */
    prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);

    UTF_TEST_CALL_MAC(prvTgfBrgNestedCoreToCustomer);

    /* restore mirror old mode */
    prvTgfMirrorToAnalyzerForwardingModeSet(oldMode);
}



/* AUTODOC:
   Set default vlan entry 3 with portIdx members 0,2;
   Configure tagged portIdx 0 as core port;
   Configure untagged portIdx 2 as customer port;
   Send double tagged traffic with S-VLAN and
   C-VLAN tags to core port and expect tagging
   traffic with C-VLAN tag only on customer port.
   Substitute VLAN tag on cascade egress port with DSA/eDSA tag
*/
UTF_TEST_CASE_MAC(prvTgfBrgNestedCoreToCustomerEgressCascadePort)
{
    /*
        1. Set configuration
        2. Generate traffic
        3. Restore configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_PUMA2_E | UTF_PUMA3_E);

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    /* enable substitution of VLAN tag with DSA tag cascade egress port */
    prvTgfBrgNestedCoreToCustomerEgressCascadePortTestAsDsa(TGF_DSA_2_WORD_TYPE_E);

    /* Set configuration */
    prvTgfBrgNestedCoreToCustomerConfigSet();

    /* Generate traffic */
    prvTgfBrgNestedCoreToCustomerEgressCascadePortTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgNestedCoreToCustomerConfigRestore();

    /* enable substitution of VLAN tag with eDSA tag cascade egress port */
    prvTgfBrgNestedCoreToCustomerEgressCascadePortTestAsDsa(TGF_DSA_4_WORD_TYPE_E);

    /* Set configuration */
    prvTgfBrgNestedCoreToCustomerConfigSet();

    /* Generate traffic */
    prvTgfBrgNestedCoreToCustomerEgressCascadePortTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgNestedCoreToCustomerConfigRestore();

}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic untagged MRU Check:
    configure VLAN, FDB entries;
    sets the VLAN Lookup mode to CPSS_IVL_E;
    configure MRU profile and MRU value and bind it to VLAN;
    send tagged traffic with different MRU;
    check counters and verify packets bigger than MRU value will be dropped.
*/
UTF_TEST_CASE_MAC(prvTgfBrgUntaggedMruCheck)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* Set configuration */
    prvTgfBrgUntaggedMruCheckConfigSet();

    /* Generate traffic */
    prvTgfBrgUntaggedMruCheckTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgUntaggedMruCheckConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test disable STP on ingress port:
    invalidate default VLAN entry;
    configure VLAN, FDB, STP entries;
    send tagged traffic;
    check counters and verify when disable STP on ingress port,
    all traffic (include BPDUs) is forwarded and learning is performed.
*/
UTF_TEST_CASE_MAC(prvTgfBrgIngressFilteringDisableStp)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* Set configuration */
    prvTgfBrgIngressFilteringDisableStpConfigSet();

    /* Generate traffic */
    prvTgfBrgIngressFilteringDisableStpTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgIngressFilteringDisableStpConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test disable STP on ingress port:
    invalidate default VLAN entry;
    configure VLAN, FDB, STP entries;
    change different STP states;
    send 2 packets with different DA;
    check counters and verify that changing STP state works properly.
*/
UTF_TEST_CASE_MAC(prvTgfBrgStpEgressFilteringChangeStates)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* Set configuration */
    prvTgfBrgStpEgressFilteringChangeStatesConfigSet();

    /* Generate traffic */
    prvTgfBrgStpEgressFilteringChangeStatesTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgStpEgressFilteringChangeStatesConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test receiving packets in cntSet1 in all modes:
    configure VLAN, FDB entries;
    set different Bridge Ingress counters modes;
    send traffic and verify that packets should discard due to:
    - FDB command.
    - Invalid SA.
    - Moved Static address is a Security Breach drop.
    - Unknown source MAC command drop, and Unknown source MAC is
      Security breach.
*/
UTF_TEST_CASE_MAC(prvTgfBrgIngressCountersSecFilterDisc)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* Set configuration */
    prvTgfBrgIngressCountersSecFilterDiscConfigSet();

    /* Generate traffic */
    prvTgfBrgIngressCountersSecFilterDiscTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgIngressCountersSecFilterDiscConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test specific MAC DA and SA monitoring by Host counter groups:
    configure VLAN, FDB entries;
    Verify that:
    -Packets with SA match are counted in field < hostOutPkts>
    -Packets with DA match are counted in field < hostInPkts>
    -Packets with BC DA are counted in fields
        < hostOutBroadcast Pkts>  & < hostOutPkts >
    -Packets with MC DA are counted in fields
        < hostOutMulticastPkts>  & < hostOutPkts >
*/
UTF_TEST_CASE_MAC(prvTgfBrgHostCounters)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfBrgHostCountersConfigSet();

    /* Generate traffic */
    prvTgfBrgHostCountersTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgHostCountersConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test specific MAC DA and SA monitoring by Matrix counter groups:
    configure VLAN, FDB entries;
    Verify that only packets with DA & SA match are counted by matrix counters;
    All other cases - DA or SA match only, or not match at all - not counted.
*/
UTF_TEST_CASE_MAC(prvTgfBrgMatrixCounters)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfBrgMatrixCountersConfigSet();

    /* Generate traffic */
    prvTgfBrgMatrixCountersTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgMatrixCountersConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify that packets that are dropped due to
    Spanning Tree state drop are counted in field<LocalPropDisc>
    in counter-set0 and counter-set1 on different modes.

    configure VLAN, FDB entries, STP states;
    bind this vlan to stp id 1;
    configure ingress counters mode;
    send traffic and verify drop counters.
*/
UTF_TEST_CASE_MAC(prvTgfBrgIngressCountersSpanningTreeStateDrop)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/


    /* Set configuration */
    prvTgfBrgIngressCountersSpanningTreeStateDropConfigSet();

    /* Generate traffic */
    prvTgfBrgIngressCountersSpanningTreeStateDropTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgIngressCountersSpanningTreeStateDropConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify receiving unknown packets in <outMcFrames> field:

    configure VLAN, FDB entries, QOS profile;
    configure ingress counters mode;
    send unknown traffic and verify drop counters.
*/
UTF_TEST_CASE_MAC(prvTgfBrgEgressCountersUnknownPacketsTcDp)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                        UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgEgressCountersUnknownPacketsTcDpConfigSet();

    /* Generate traffic */
    prvTgfBrgEgressCountersUnknownPacketsTcDpTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgEgressCountersUnknownPacketsTcDpConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Verify mirrored to analyzer traffic in <outCtrlFrames> field:

    configure VLAN, FDB entries;
    configure analyzer interface;
    send traffic and verify drop counters.
*/
UTF_TEST_CASE_MAC(prvTgfBrgEgressCountersCtrlToAnlyzrPortVlan)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* SIP-6 devices don't support daMirrorToRxAnalyzerPortEn in FDB entry */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E);

    /* Set configuration */
    prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigSet();

    /* Generate traffic */
    prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Execute basic test running Address Update (AU) queue workaround
                   for single and double AU queue

    send 2K + 64 new MAC addresses to the hardware MAC address;
    disable sending AA and TA messages to AUQ;
    start flush FDB triggred action - action fail;
    execute WA;
    Start flush FDB triggred action - action finished;
    enable sending AA and TA messages to AUQ;
*/
UTF_TEST_CASE_MAC(prvTgfFdbStuckAuqWaBasic)
{
    /* Check if the device supports the AU queue WA */
    if(GT_FALSE == prvTgfFdbAuWaCheck(prvTgfDevNum, GT_FALSE))
    {
        return ;
    }

    /* Run test */
    prvTgfFdbAuStuckWaBasicAuqTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Execute comprehensive test running Address Update (AU) queue workaround
                   for double AU queue

    run basic WA test in cycle learning different MAC addresses ranges;
    check primary and secondary AUQ management and order of incoming messages.

*/
UTF_TEST_CASE_MAC(prvTgfFdbStuckDoubleAuqWa)
{
    /* Check if the device supports the AU queue WA for double AUQ */
    if(GT_FALSE == prvTgfFdbAuWaCheck(prvTgfDevNum, GT_TRUE))
    {
        return ;
    }
    /* Run test */
    prvTgfFdbAuStuckWaDoubleQueueTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Check basic Jumbo Frame functionality:
    configure VLAN with all tagged ports;
    configure Jumbo Frame settings;
    send unknown UC and BC packets with size more 1.5K;
    verify output traffic flooded on all ports in Vlan.
*/
UTF_TEST_CASE_MAC(prvTgfBrgBasicJumboFrame)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

#ifndef ASIC_SIMULATION
#ifdef CHX_FAMILY
    GT_BOOL  useMii; /* do we used MII or SDMA */

    useMii = ((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE:
              (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE:
               PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfCpuDevNum) ? GT_TRUE : GT_FALSE);

    if (GT_TRUE == useMii)
    {
        PRV_UTF_LOG1_MAC("\n >>> Skipped due to kernel panic. useMii = %d\n\n", useMii);

        /* mark test as skipped */
        prvUtfSkipTestsSet();
        return;
    }
#endif /* CHX_FAMILY */
#endif /* ASIC_SIMULATION */

#ifdef ASIC_SIMULATION
#ifdef EXMXPM_FAMILY
        PRV_UTF_LOG0_MAC("\n >>> Skipped due to Golden Module Problem\n");

        /* mark test as skipped */
        prvUtfSkipTestsSet();
        return;
#endif /* EXMXPM_FAMILY */
#endif /* ASIC_SIMULATION */

    /* Set configuration */
    prvTgfBrgBasicJumboFrameConfigSet();

    /* Generate traffic */
    prvTgfBrgBasicJumboFrameTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgBasicJumboFrameConfigRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN members tables access based on VID1 indexing.

   Verify that traffic is flooded to or filtered according to the
   VLAN Members entry indexed by the configured mode - VLAN or VID1.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanMembersIndexingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfBrgBrgVlanMembersIndexingConfigurationSet();

    /* Generate traffic */
    prvTgfBrgBrgVlanMembersIndexingTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgBrgVlanMembersIndexingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN STG based on VID1 indexing.

   Verify that traffic is forwarded or blocked according to the
   VLAN STG indexed by the configured mode - VLAN or VID1.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanStgIndexingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfBrgBrgVlanStgIndexingConfigurationSet();

    /* Generate traffic */
    prvTgfBrgBrgVlanStgIndexingTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgBrgVlanStgIndexingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test per VLAN, per Port egress tag state based on VID1 indexing.

   Verify that traffic egress from the same port, has different tagging (double
   tagged or untagged) from Egress Tag State entry accessing based on
   VLAN or VID1 value.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanTagStateIndexingMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Set configuration */
    prvTgfBrgBrgVlanTagStateIndexingConfigurationSet();

    /* Generate traffic */
    prvTgfBrgBrgVlanTagStateIndexingTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgBrgVlanTagStateIndexingConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test new DSA assignment for to CPU.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanForceNewDsaToCpu)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_AC3X_E);

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        SKIP_TEST_MAC
    }

    /* Set configuration */
    prvTgfBrgVlanForceNewDsaToCpuConfigurationSet();

    /* Generate traffic */
    prvTgfBrgVlanForceNewDsaToCpuTrafficGenerate();

    /* Restore configuration */
    prvTgfBrgVlanForceNewDsaToCpuConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Bridge Bypass Modes & verify MAC SA learning
    Configure PCL and FDB entries.
    No Bridge Bypass - Forwarding decision & packet command according to
    FDB, verify MAC SA learning.
    Set Bridge Bypass mode to SA learning only - Forwarding decision &
    packet command according to PCL, verify MAC SA learning.
    Set Bridge Bypass mode to only Forwarding decision- Forwarding decision
    according to PCL, packet command according to FDB, verify MAC SA learning.
*/

UTF_TEST_CASE_MAC(prvTgfBrgGenBypassMode)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfBrgGenBypassModeTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic target ePort MTU Check:
    test for every profile id [0..3]
      configure FDB entry
      configure MTU profile id and MTU value and bind it to ePort
      configure MTU command to DROP
      send untagged traffic with different MTU
      check counters and verify packets bigger than MTU value will be dropped.
      configure MTU command to FORWARD
      send untagged traffic with different MTU
      check counters and verify packets bigger than MTU value will be forwarded.
      configure MTU command to MIRROR to CPU
      configure CPU code exception to NET control
      send untagged traffic with different MTU
      check counters and verify packets bigger than MTU value will be received
      in CPU with correct code.
*/
UTF_TEST_CASE_MAC(prvTgfBrgGenMtuCheck)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;
    GT_U32      profileId;

    /* feature not supported in WM therefore skip test
       - to be removed after adding support*/
    /*ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC*/

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Supported only by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    for (profileId = 0; profileId < 4; profileId++) {
        PRV_UTF_LOG1_MAC("\n==== Start test for profile=%d ==========\n\n",profileId);

        /* Set configuration */
        prvTgfBrgGenMtuCheckConfigSet(profileId);

        /* Generate traffic */
        prvTgfBrgGenMtuCheckTrafficGenerate();

        /* Restore configuration */
        prvTgfBrgGenMtuCheckConfigRestore(profileId);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - eDSA tag handling with double tag:
    Test the various egress eDSA port status.
*/
UTF_TEST_CASE_MAC(prvTgfVlanEDsaTagHandling)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    prvTgfVlanEDsaTagHandlingTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - check ARP trapping configuration:

   set configuration:
       - enable  IP control traffic trapping to CPU
       - enable trapping to CPU ARP Broadcast packets for all VLANs
       - set a arp broadcast mode
   send an broadcast ARP packet
   check that packet received correctly
.
*/
UTF_TEST_CASE_MAC(prvTgfArpTrapping)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,  UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
                                        UTF_PUMA2_E | UTF_PUMA3_E);
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    prvTgfArpTrappingTest();
}

static  void internal_prvTgfBrgVlanTagPopTtiTest
(
    IN PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_ENT numBytesToPop
)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    PRV_UTF_LOG1_MAC("prvTgfBrgVlanTagPopTtiTest : [%d] tags , part with reassign EPort \n", numBytesToPop);
    /* test with TTI action + L2 re-parsing for tags after the pop */
    prvTgfBrgVlanTagPopTtiTest(numBytesToPop,GT_TRUE);

    PRV_UTF_LOG1_MAC("prvTgfBrgVlanTagPopTtiTest : [%d] tags , part without reassign EPort \n",numBytesToPop);
    /* test without TTI action so no L2 re-parsing */
    prvTgfBrgVlanTagPopTtiTest(numBytesToPop,GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Ingress removing 0 VLAN tag words.
            The correct TTI removing.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanTagPop0TtiTest)
{
    internal_prvTgfBrgVlanTagPopTtiTest(PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E);
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Ingress removing 1 VLAN tag words.
            The correct TTI removing.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanTagPop1TtiTest)
{
    internal_prvTgfBrgVlanTagPopTtiTest(PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_4_E);
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Ingress removing 2 VLAN tag words.
            The correct TTI removing.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanTagPop2TtiTest)
{
    internal_prvTgfBrgVlanTagPopTtiTest(PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_8_E);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test -  Cisco L2 protocol / Proprietary Layer 2 control MC command.
    set the command to FORWARD, then MIRROR_TO_CPU, and then TRAP_TO_CPU.
    for each setting, generate proprietary L2 ctrl packet,
    and check the packet is handled correctly by the PP.

*/

UTF_TEST_CASE_MAC(prvTgfBrgGenPropL2ControlProtCmd)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,  UTF_PUMA2_E | UTF_PUMA3_E);
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    prvTgfBrgGenPropL2ControlSet();
    prvTgfBrgGenPropL2ControlTest();
    prvTgfBrgGenPropL2ControlRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test -  Cisco L2 protocol / Proprietary Layer 2 control MC command.
    set the command to FORWARD, then MIRROR_TO_CPU, and then TRAP_TO_CPU.
    for each setting, generate proprietary L2 ctrl packet,
    and check the packet is handled correctly by the PP.
    using AutoFlow
*/

UTF_TEST_CASE_MAC(prvTgfBrgGenPropL2ControlProtCmdAf)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,  UTF_PUMA2_E | UTF_PUMA3_E);
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    prvTgfBrgGenPropL2ControlSetAf();
    prvTgfBrgGenPropL2ControlTestAf();
    prvTgfBrgGenPropL2ControlRestoreAf();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Vid1 field in the learned FDB entry.
   Using AutoFlow
*/

UTF_TEST_CASE_MAC(prvTgfBrgGenVid1InLearnFDBCheck)
{
        GT_U32 notAppFamilyBmp;

        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
        PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    prvTgfBrgGenVidInLearnFDBCheckTrafficGeneratorAf();
    prvTgfBrgGenVidInLearnFDBCheckRestoreAf();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test -  IPv6 ICMP command.
    enable ICMP TO CPU per VLAN.
    set the command to FORWARD/MIRROR/TRAP for different msgTypes.
    generate ICMPv6 packet of each msgType,
    and check the packet is handled correctly by the PP.
    disable the feature.
    generate ICMPv6 packets of msgTypes associated with MIRROR and TRAP cmds,
    and check the packets are FORWARDED by the PP.
  */

UTF_TEST_CASE_MAC(prvTgfBrgGenIcmpV6)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,  UTF_PUMA2_E | UTF_PUMA3_E);
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    prvTgfBrgGenIcmpV6Set();
    prvTgfBrgGenIcmpV6Test();
    prvTgfBrgGenIcmpV6Restore();
}

/* AUTODOC: basic VPLS configuration of device with 2 ports in MPLS domain , and 2 ports of different service providers.
    1. test check 'flooding' from service provider port to the other 3 ports.(TS to the MPLS domain)
        a. check SA learning (reassigned src eport)
    2. test check 'flooding' from mpls domain port only to service provider ports (filter the other MPLS domain port). (TT of ingress packet - Ethernet passenger)
        a. check SA learning (reassigned src eport)
    3. test 'Known UC' from service provider port to MPLS domain.
    3. test 'Known UC' from MPLS domain port to service provider port.


    presentation of system is in Documentum :
    Cabinets/SWITCHING/CPSS and PSS/R&D Internal/CPSS/sip5 device (Lion3,BC2)/Test design/VPLS/Bobcat2_VPLS_Demo.pptx
    http://docil.marvell.com/webtop/drl/objectId/0900dd88801954ba

    ePort A is assigned for EFP1 - port to Ethernet domain - service provider 1 with outer vlan tag 10
    ePort B is assigned for EFP2 - port to Ethernet domain - service provider 2 with outer vlan tag 20
    ePort C is assigned for PW1  - port to MPLS domain , LABEL 50 - PW port 1 - PW label 60
    ePort D is assigned for PW2  - port to MPLS domain , LABEL 50 - PW port 2 - PW label 70

    eVlan - VPLS domain - the eVlan that represents the 'VPLS domain'
*/
UTF_TEST_CASE_MAC(prvTgfBrgVplsBasicTest)
{
        GT_U32 notAppFamilyBmp;

        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
        PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    /* skip this test for all families under Golden Model */
    /* the test send 9 packets (with TCAM lookups) that takes the GM 360 seconds
       on Linux , and xxx on WIN32 */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfBrgVplsBasicTest();

}
/* AUTODOC: similar to file prvTgfBrgVplsBasicTest but with next changes:
1. supports 3 modes :
   a. 'pop tag' - the mode that is tested in prvTgfBrgVplsBasicTest
   b. 'raw mode' + 'QinQ terminal'(delete double Vlan)
   c. 'tag mode' + 'add double vlan tag'
2. in RAW mode ethernet packets come with 2 tags and the passenger on MPLS
   tunnels is without vlan tags.
3. in TAG mode the ethernet packets come with one tag but considered untagged
   when become passenger on MPLS tunnels and so added additional 2 vlan tags.
*/
UTF_TEST_CASE_MAC(prvTgfBrgVplsBasicTest1)
{
        GT_U32 notAppFamilyBmp;

        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
        PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfBrgVplsBasicTest1();
}

/* AUTODOC: - test on VLAN member add/remove operation atomicity
    the possible problem is leaking of several packets with/wihout VLAN Tag
    not as defined in membership of added/removed port.
    - Initial configuration
    - add/remove TX port as tagged VLAN member under Wire Speed traffic
       1024 times and check sent packets and sent octets amounts
    - add/remove TX port as untagged VLAN member under Wire Speed traffic
       1024 times and check sent packets and sent octets amounts
    -  Restore configuration
*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanFwsTest)
{
#ifdef ASIC_SIMULATION
        SKIP_TEST_MAC
#else
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_DXCH_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    prvTgfBrgVlanFwsTest();
#endif
}

UTF_TEST_CASE_MAC(prvTgfBrgPhysicalPortsSetUpTest)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfBrgPhysicalPortsSetUpTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfFdbIpv4v6Uc2VrfRouting:
    Description:
        This test creates
            VLAN 5 with VRFid 4 and enabled routing for IPv4 packet
            VLAN 6 with VRFid 6 and enabled routing for IPv6 packet.
    The goal is to:
        send IPv4 packet to VLAN 5 and get it routed with VRFid 4 to VLAN 6
        send IPv6 packet to VLAN 6 and get it routed with VRFid 6 to VLAN 5

    ======= build IPv4 and IPv6 Packets =======

    ======= Set vlans configuration =======
        create VLAN 5 with tagged ports [0,1]
        create VLAN 6 with tagged ports [2,3]

    ======= Set generic routing configuration =======

    ======= Set FDB routing configuration =======

    ======= Send traffic to VLAN 5. Expect routed packet to VLAN 6 =======

    ======= Send traffic to VLAN 6. Expect routed packet to VLAN 5 =======

    ===== test configuration restore =====
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4v6Uc2VrfRouting)
{

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E);

    prvTgfFdbIpv4v6Uc2VrfRouting();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfFdbIpv4UcRoutingMtuCheckNonDf:
    Description:
        The purpose of this test is to check the functionality
        of MTU profile index on routing with fragmented (Non_DF) packets

    ======= build IPv4 Packet =======

    ======= Set vlans configuration =======
        create VLAN 5 with tagged ports [0,1]
        create VLAN 6 with tagged ports [2,3]

    ======= Set generic routing configuration =======

    ======= Set FDB routing configuration =======

    ======= Send traffic to VLAN 5. Expect routed packet to VLAN 6 =======

    ======= Change MTU profile in FDB entry wit MTU exceeding packet size =======

    ======= Send traffic to VLAN 5. Expect Packet dropped =======

    ===== test configuration restore =====
*/
UTF_TEST_CASE_MAC(prvTgfFdbIpv4UcRoutingMtuCheckNonDf)
{

    GT_U32      notAppFamilyBmp;

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfFdbIpv4UcRoutingMtuCheckNonDf();
}


/* functional FDB learning (and AUQ messaging) test after 'gtShutdownAndCoreRestart' */
/* this also test the 'FROM_CPU' after 'gtShutdownAndCoreRestart' */
UTF_TEST_CASE_MAC(tgfBasicDynamicLearning_after_gtShutdownAndCoreRestart)
{
    GT_STATUS   st;
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    if(prvTgfResetModeGet_gtShutdownAndCoreRestart() == GT_FALSE)
    {
        /* not applicable device */
        prvUtfSkipTestsSet();
        return;
    }

    if(GT_TRUE == prvTgfResetModeGet())
    {
        /* run the 'tgfBasicDynamicLearning' without doing HW reset at the end ...
           only 'clean up' of configurations .

           because we want to run 'tgfBasicDynamicLearning' again after 'gtShutdownAndCoreRestart'

           and we need to avoid the HW reset because we want the AUQ to NOT start
           from index 0
        */

        prvTgfResetModeSet(GT_FALSE); /* force to not do 'HW reset' at the end */
        UTF_TEST_CALL_MAC(tgfBasicDynamicLearning);
        prvTgfResetModeSet(GT_TRUE);/* restore value */
    }
    else
    {
        UTF_TEST_CALL_MAC(tgfBasicDynamicLearning);
    }

    /* do the 'gtShutdownAndCoreRestart' */
    st = prvTgfResetAndInitSystem_gtShutdownAndCoreRestart();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvTgfResetAndInitSystem_gtShutdownAndCoreRestart : FAILED st=[%d]",st);
    if(st != GT_OK)
    {
        /* no more */
        return;
    }

    /* this time is after 'gtShutdownAndCoreRestart' and allowing 'HW reset'
       at the end of 'tgfBasicDynamicLearning' */
    UTF_TEST_CALL_MAC(tgfBasicDynamicLearning);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Performance of ASIC simulation:
 * 1. Wire speed of UC traffic w/o flood
 * 2. Wire speed of UC traffic w flood
 */

UTF_TEST_CASE_MAC(prvTgfBrgAsicSimulationPerformance)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,  (~UTF_DXCH_E));
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    GM_NOT_SUPPORT_THIS_TEST_MAC
#ifdef ASIC_SIMULATION
    prvTgfBrgAsicSimulationPerformanceConfigSet();
    prvTgfBrgAsicSimulationPerformanceTrafficGenerate();
    prvTgfBrgAsicSimulationPerformanceConfigRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Check Sip5.20 VLAN entry invalidation.
    Sip5.20 has no valid bit on Egress VLAN - so VLAN entry invalidation may
    works uncorrectly without port members reset.

    Configuration:
        1. Create VLAN entry 5 and add port 1 to it.
        2. Create VLAN entry 6 and add port 2 to it.
        3. Add route:
           Ingress VLAN 5, port 1  ->  Egress VLAN 6, port 2

    Traffic:
        1. Transmit frame to port 1.
        2. Invalidate VLAN entry 6 and transmit frame to port 1.

    Expected:
        1. Frame was sent from port 2.
        2. Frame was NOT sent from port 2.
*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(prvTgfBrgVlanInvalidate)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
            UTF_LION2_E | UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC3X_E | UTF_CAELUM_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* configuration set */
    prvTgfBrgVlanInvalidateConfigurationSet();

    /* generate traffic */
    prvTgfBrgVlanInvalidateTrafficGenerate();

    /* restore configuration */
    prvTgfBrgVlanInvalidateConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Check PP housekeeping tasks under FWS

   1. Check FDB access under FWS
   2. Check trigger aging and automatic aging before FWS, under FWS and after FWS
   3. Check FDB flush before FWS, under FWS and after  FWS
   4. restore

*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(prvTgfBrgFDBFwsTraffic)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_PUMA_E | UTF_PUMA2_E | UTF_LION2_E | UTF_XCAT3_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    #ifndef ASIC_SIMULATION
    prvTgfBrgFDBFwsTraffic();
    #endif
}

/*
 * Configuration of tgfBridge suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfBridge)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVplsBasicTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVplsBasicTest1)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgNestedCoreToCustomer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgNestedCoreToCustomer_captureUseSrcBasedMirror)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgNestedCoreToCustomerEgressCascadePort)

    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicDynamicLearning)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicDynamicLearning_fromCascadePortDsaTagForward)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTransmitAllKindsOfTraffic)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBrgVlanIngressFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBrgVlanEgressFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(tgfBrgVlanL2VpnIngressFiltering)


    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTableWriteWorkaround)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStgTableWriteWorkaround)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbTableReadUnderWireSpeed)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationUntaggedCmdDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationUntaggedCmdDoubleTag_withErrorInjection)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationUntaggedCmdSingleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationUntaggedCmdUntag)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationTag0CmdDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationTag0CmdSingleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationTag0CmdUntag)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationTag1CmdDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationTag1CmdSingleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationTag1CmdUntag)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdDoubleTag_egressCascadePort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdSingleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag0InnerTag1CmdUntag)


    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdDoubleTag_egressCascadePort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdSingleTag_egressCascadePort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationOuterTag1InnerTag0CmdUntag)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationPushTag0CmdDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationPushTag0CmdSingleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationPushTag0CmdUntag)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationPopOuterTagCmdDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationPopOuterTagCmdSingleTag)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationIngressVlanAssignment1)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationIngressVlanAssignment2)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationChangeEgressTpid)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanManipulationEthernetOverMpls)

    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanFloodVidx)
    UTF_SUIT_DECLARE_TEST_MAC(tgfVlanFloodVidxAllFlooded)

    UTF_SUIT_DECLARE_TEST_MAC(tgfFdbAgeBitDaRefresh)
    UTF_SUIT_DECLARE_TEST_MAC(tgfFdbAgingInMultiPortGroupDevice)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbAuMessageFiltering)

    UTF_SUIT_DECLARE_TEST_MAC(tgfBrgGenIeeeReservedMcastProfiles)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenBypassMode)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgSrcIdPortForce)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgSrcIdPortForce_bypassIngressPipe)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgSrcIdFdbSaAssigment)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgSrcIdFdbDaAssigment)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTag1RemIfZeroDisabled)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTag1RemIfZeroIngrPktSingleTagged)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTag1RemIfZeroIngrPktDoubleTagged)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTag1RemIfZeroIngrPktUntagged)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTag1RemIfZeroDoubleTagDisable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanTag1RemIfZeroTag1AssignmentIsNotZero)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanKeepVlan1EnTagStateVlan1Vlan0DoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanKeepVlan1EnTagStateVlan0DoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanKeepVlan1EnTagStateUntaggedDoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanKeepVlan1EnTagStateVlan0SingleTag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanKeepVlan1DisTagStateVlan0DoubleTag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanKeepVlan1DisTagStateUntaggedDoubleTag)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenIeeeReservedMcastLearningEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenIeeeReservedMcastLearningDisable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenIeeeReservedMcastLearningDisableTrapDisable)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenRateLimitMcastRegEnableRegPkts)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenRateLimitMcastRegDisableRegPkts)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenRateLimitMcastRegEnableUnregPkts)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenRateLimitMcastRegDisableUnregEnableRegPkts)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbAuNaMessageSpUnknown)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbAuNaMessageExtFormatWithTag1DueToNewAddr)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbAuNaMessageExtFormatDueToMovedPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbAuNaMessageExtFormatDueToMovedTrunk)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbAuNaMessageExtFormatFdbHashIndex)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenRateLimitStackPortWsTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanPortAccFrameType)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgProtocolBasedVlanLlc)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgInvalidVlanFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgEgrTagModifNonDsaTaggedEgrPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgNestedCustomerToCore)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgNestedCustomerToCoreEgressCascadePort)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbChangeInterfaceType)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbChangeDaCommand)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbChangeSaCommand)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbTriggeredAgingEntries)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbTriggeredAgingVlans)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbDelMacEntriesForVlanPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMcGroupRxMirror)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbSourceIdModeDefault)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbSourceIdModeChange)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgCustomerBpdus)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgProviderBpdus)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbLearningAdressesSvlMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMcBridgingIpV4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMcBridgingIpV6)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgDroppingIpV6MacMulticastFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgDroppingNonIpV4MacMulticastFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbPerInlifPortUnregIpv6McFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbPerInlifPortUnregIpv4BcFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbSecurityBreach)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbPerInlifPortUnknownUcFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbPerEgressPortUnknownUcFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbPerEgressPortUnregIpv4McFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbPerEgressPortUnregIpv6BcFiltering)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbLocalUcSwitching)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbLocalMcSwitching)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbBasicIpv4UcRoutingByMsg)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbBasicIpv4UcRoutingByIndex)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcPointerRouteByIndex)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcPointerRouteByMsg)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcRoutingLookupMask)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbBasicIpv4UcRoutingVlanModeIpCounters)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbBasicIpv4UcRoutingFillFdbByIndex)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbBasicIpv4UcRoutingFillFdbByMessage)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcRoutingDeleteEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcRoutingRefreshEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcRoutingTransplantEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcRoutingAgingEnable)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbBasicIpv6UcRouting)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv6UcRoutingLookupMask)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv6UcRoutingDeleteEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv6UcRoutingRefreshEnable)
    /*UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv6UcRoutingTransplantEnable)*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv6UcRoutingAgingEnable)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgEgressCountersUnknownPacketsTcDp)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgIngressCountersSpanningTreeStateDrop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgMatrixCounters)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgHostCounters)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgEgressCountersCtrlToAnlyzrPortVlan)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgUntaggedMruCheck)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgIngressFilteringDisableStp)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgStpEgressFilteringChangeStates)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgIngressCountersSecFilterDisc)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbStuckAuqWaBasic)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbStuckDoubleAuqWa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgBasicJumboFrame)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbSecureAutoLearnMovedSa)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanMembersIndexingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanStgIndexingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanTagStateIndexingMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanForceNewDsaToCpu)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenMtuCheck)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVlanEDsaTagHandling)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfArpTrapping)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanTagPop0TtiTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanTagPop1TtiTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanTagPop2TtiTest)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenPropL2ControlProtCmd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenPropL2ControlProtCmdAf)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenVid1InLearnFDBCheck)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgGenIcmpV6)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4v6Uc2VrfRouting)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFdbIpv4UcRoutingMtuCheckNonDf)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanFwsTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgPhysicalPortsSetUpTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgVlanInvalidate)

    UTF_SUIT_DECLARE_TEST_MAC(tgfBasicDynamicLearning_after_gtShutdownAndCoreRestart)

    /* Performance tests */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFDBFwsTraffic)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgAsicSimulationPerformance)

UTF_SUIT_END_TESTS_MAC(tgfBridge)
