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
* @file tgfCommonPortUT.c
*
* @brief Enhanced UTs for CPSS Port
*
* @version   26
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <port/prvTgfPortMacCountersCapture.h>
#include <port/prvTgfPortTxResourceHistogram.h>
#include <port/prvTgfPortTx.h>
#include <port/prvTgfPortIfModeSpeed.h>
#include <port/tgfPortFlowControlPacketsCounter.h>
#include <port/prvTgfPortMruCheck.h>
#include <port/tgfPortFwdToLb.h>
#include <port/prvTgfPortTxTailDropDba.h>
#include <port/prvTgfPortStatMcFilterDropPkts.h>

#include <trafficEngine/tgfTrafficTable.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port MAC BC capture counters:
    configure VLAN, FDB entries;
    send BC traffic;
    verify updated MAC couners for BC traffic.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibBroadcastCaptureCntr)
{
/*
    1. Capture Broadcast traffic
 */
    prvTgfPortMacCountersVlanConfigurationSet();
    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterCaptureBroadcastTrafficGenerate();
    prvTgfPortMacCountersRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port MAC UC capture counters:
    configure VLAN, FDB entries;
    send UC traffic;
    verify updated MAC couners for UC traffic.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibUnicastCaptureCntr)
{
/*
    1. Capture Unicast traffic
 */
    prvTgfPortMacCountersVlanConfigurationSet();
    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterCaptureUnicastTrafficGenerate();
    prvTgfPortMacCountersRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port MAC MC capture counters:
    configure VLAN, FDB, VIDX entries;
    send MC traffic;
    verify updated MAC couners for MC traffic.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibMulticastCaptureCntr)
{
/*
    1. Capture Multicast traffic
 */
    prvTgfPortMacCountersVlanConfigurationSet();
    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterCaptureMulticastTrafficGenerate();
    prvTgfPortMacCountersRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port MAC oversize capture counters:
    configure VLAN, FDB entries;
    configure Rx MRU for send port;
    send UC traffic with bigger than MRU size;
    verify no Tx traffic and updated MAC oversize couners.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibOversizeCaptureCntr)
{
/*
    1. Capture Oversize traffic
 */
#if defined ASIC_SIMULATION
    /* in Puma GM - this test kills GM - GM bug ! */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
#else
    /* the Puma HW is ok with this test */
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_PUMA_E);
#endif

        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E, CPSS-6063);
    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   0,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }


    prvTgfPortMacCountersVlanConfigurationSet();
    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterCaptureOversizeTrafficGenerate();
    prvTgfPortMacCountersRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port MAC UC capture counters for traffic from CPU:
    configure VLAN, FDB entries;
    enable\disable\enable CPU port;
    send UC traffic verify updated MAC UC couners.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibCpuTrafficCaptureCntr)
{
    GT_STATUS rc;   /* return code */
    GT_BOOL   cpuPortDisableCheck; /* do check of CPU port disabled case */

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E |UTF_PUMA_E))
    {
        /* there is no dedicated CPU MAC for devices */
        cpuPortDisableCheck = GT_FALSE;
    }
    else
    {
        cpuPortDisableCheck = GT_TRUE;
    }

    prvTgfPortMacCountersVlanConfigurationSet();

    /* check unicast from CPU with CPU port enabled */
    if (cpuPortDisableCheck)
    {
        rc = prvTgfPortEnableSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortEnableSet: %d, %d, GT_TRUE",
                                     prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);
    }

    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterCaptureUnicastTrafficGenerate();
    prvTgfPortMacCountersRestore();

    /* check unicast from CPU with CPU port disabled */
    if (cpuPortDisableCheck)
    {
        prvTgfPortMacCountersConfigurationSet();
        prvTgPortMacCounterCaptureCpuPortDisable();
        prvTgfPortMacCountersRestore();

        /* again enable CPU port and check unicast from CPU */
        rc = prvTgfPortEnableSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortEnableSet: %d, %d, GT_TRUE",
                                     prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);
    }
    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterCaptureUnicastTrafficGenerate();
    prvTgfPortMacCountersRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port MAC UC capture counters for traffic to CPU:
    configure VLAN with TRAP_TO_CPU cmd, FDB entries;
    enable\disable\enable CPU port;
    send UC traffic;
    verify updated MAC couners for enabled CPU port, no traffic - disable CPU.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibToCpuTrafficCaptureCntr)
{
    GT_STATUS rc;   /* return code */
    GT_BOOL   cpuPortDisableCheck; /* do check of CPU port disabled case */

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS |UTF_PUMA_E))
    {
        /* there is no dedicated CPU MAC for devices */
        cpuPortDisableCheck = GT_FALSE;
    }
    else
    {
        cpuPortDisableCheck = GT_TRUE;
    }

    prvTgfPortMacCountersToCpuVlanConfigurationSet();

    /* enable CPU port and check traffic to CPU */
    if (cpuPortDisableCheck)
    {
        rc = prvTgfPortEnableSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortEnableSet: %d, %d, GT_TRUE",
                                     prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);
    }

    prvTgfPortMacCountersConfigurationSet();
    prvTgfPortToCpuTraffic(GT_TRUE);
    prvTgfPortMacCountersRestore();

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTableRxPcktTblClear FAILED, rc = [%d]", rc);
        return;
    }

    if (cpuPortDisableCheck)
    {
        /* disable CPU port and see no traffic to CPU (if MII interface used) */
        rc = prvTgfPortEnableSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortEnableSet: %d, %d, GT_TRUE",
                                     prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);
        prvTgfPortMacCountersConfigurationSet();
        prvTgfPortToCpuTraffic(GT_FALSE);
        prvTgfPortMacCountersRestore();

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTableRxPcktTblClear FAILED, rc = [%d]", rc);
            return;
        }

        /* enable CPU port again and check traffic to CPU */
        rc = prvTgfPortEnableSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortEnableSet: %d, %d, GT_TRUE",
                                     prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);
    }

    prvTgfPortMacCountersConfigurationSet();
    prvTgfPortToCpuTraffic(GT_TRUE);
    prvTgfPortMacCountersRestore();

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTableRxPcktTblClear FAILED, rc = [%d]", rc);
        return;
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port MAC histogram capture counters:
    configure VLAN, FDB entries;
    enable\disable Rx\Tx Histogram counters;
    send UC traffic with different size;
    verify updated MAC couners when feature is enabled;
    verify not updated MAC couners when feature is disabled.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibHistogramCaptureCntr)
{
/*
    1. Generate Unicast traffic and check Histogram counters
    2. Restore Defult Settings
 */
#if (defined LINUX_SIM)
    /* this test cause the linux GM simulation to crash */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
#endif

        PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E, CPSS-6063);

        prvTgfPortMacCountersVlanConfigurationSet();
    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterCaptureHistogramTrafficGenerate();
    prvTgfPortMacCountersRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port clear-on-read MAC counters:
    configure VLAN, FDB entries;
    enable\disable clear-on-read counters feature;
    send UC traffic;
    verify cleared MAC couners when feature is enabled;
    verify not cleared on read MAC couners when feature is disabled.
*/
UTF_TEST_CASE_MAC(tgfPortMacMibClearOnReadCaptureCntr)
{
/*
    1. Generate Unicast traffic and Check Clear on Read feature
    2. Restore Defult Settings
 */
    prvTgfPortMacCountersVlanConfigurationSet();
    prvTgfPortMacCountersConfigurationSet();
    prvTgPortMacCounterClearOnReadTrafficGenerate();
    prvTgfPortMacCountersRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port Tx resource histogram counters:
    configure VLAN, FDB entries;
    configure threshold for Histogram counter increment;
    configure tail drop profile and TC limits;
    enable\disable transmission from TC queue;
    send UC traffic and verify Tx traffic and histogram counters.
*/
UTF_TEST_CASE_MAC(tgfPortTxResourceHistogramCntr)
{
/*
    1. Generate Unicast traffic and Check feature
    2. Restore Defult Settings
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum,
          UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
          UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_PUMA_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    prvTgfPortTxResourceHistogramVlanConfigurationSet();
    prvTgfPortTxResourceHistogramConfigurationSet();
    prvTgfPortTxResourceHistogramTrafficGenerate();
    prvTgfPortTxResourceHistogramRestore();
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test MAC Counters read by packet on applicable devices:
    1. Configure appDemo DB to use the MAC Counters read by packet feature;
    2. Restart CPSS;
    3. Execute some test exercising MAC Capture trigger / capture;
    4. Restore.
*/
UTF_TEST_CASE_MAC(tgfPortRemotePortsMacCountersByPacket)
{
/*
    1. Configure appDemo DB to use the MAC Counters read by packet feature;
    2. Restart CPSS;
    3. Execute some test exercising MAC Capture trigger / capture;
    4. Restore.
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_AC3X_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    {
        GT_U32      useMibCounterReadByPacketSave = 0;

        appDemoDbEntryGet("useMibCounterReadByPacket", &useMibCounterReadByPacketSave);
        appDemoDbEntryAdd("useMibCounterReadByPacket",(GT_U32) GT_TRUE);
        prvTgfResetAndInitSystem();
        UTF_TEST_CALL_MAC(tgfPortMacMibUnicastCaptureCntr);
        UTF_TEST_CALL_MAC(tgfPortMacMibBroadcastCaptureCntr);
        UTF_TEST_CALL_MAC(tgfPortMacMibMulticastCaptureCntr);
        appDemoDbEntryAdd("useMibCounterReadByPacket", useMibCounterReadByPacketSave);
        prvTgfResetAndInitSystem();
    }
#endif /* ASIC_SIMULATION */
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test port Tx SP arbiter scheduler:
    configure VLAN, FDB entries;
    configure egress port scheduler to work in SP arbiter;
    generate Wire Speed UC traffic to one egress;
    verify results - traffic Tx only from highest TC queue.
*/
UTF_TEST_CASE_MAC(tgfPortTxSchedulerSp)
{
/*
    1. Configure egress port scheduler to work in SP arbiter
    2. Generate wire speed Unicast traffic from 3 ports to one egress.
       Check results. Traffic Txed only from highest TC queue.
    3. Restore Default Settings
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* scheduler in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* if not Aldrin2 the function does nothing */
    appDemoDxAldrin2TailDropDbaEnableSet(prvTgfDevNum, GT_FALSE);

    prvTgfPortTxSpSchedulerConfigurationSet();
    prvTgfPortTxSpSchedulerTrafficGenerate();
    prvTgfPortTxSpSchedulerRestore();

    /* if not Aldrin2 the function does nothing */
    appDemoDxAldrin2TailDropDbaEnableRestore(prvTgfDevNum);
#endif /* ASIC_SIMULATION */
}


/*
    tgfPortIfModeSpeed
*/
UTF_TEST_CASE_MAC(tgfPortIfModeSpeed)
{
/* Run through all ifMode's/speed's supported by port,
   send traffic in loopback see FWS, no packet drops etc.
*/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
                                        /* | UTF_XCAT2_E */ | UTF_LION2_E
                                        | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_XCAT3_E
                                        | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    CPSS_COVERITY_NON_ISSUE_BOOKMARK
    /* coverity[unreachable] */
    prvTgfPortIfModeSpeed();
#endif
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: PFC global counters check test:
       Enable PFC reception on all ports
       Send PFC packets to all ports
       Check that all packets are counted by PFC global counters
       Disable PFC reception on all ports and clean FDB
*/
UTF_TEST_CASE_MAC(tgfPortFlowControlPacketsCounter)
{
/*
    1. Generate traffic
    2. Check counters
*/
    GT_U32      notAppFamilyBmp;
    GT_STATUS   rc;
    /* Supported only by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* AUTODOC: enable PFC reception on all test ports */
    rc = prvTgfPortPFCAllPortsReceptionEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortPFCAllPortsReceptionEnableSet: %d", GT_TRUE);

    /* AUTODOC: Generate PFC traffic */
    prvTgfPortFCTrafficGenerate();

    /* AUTODOC: Check counters */
    prvTgfPortFCCounterCheck();

    /* AUTODOC: disable PFC reception on all test ports */
    rc = prvTgfPortPFCAllPortsReceptionEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortPFCAllPortsReceptionEnableSet: %d", GT_TRUE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* there is no need to restore configuration
       (no additional configuration added) */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test basic port profile MRU Check:
    test for every profile id [0..7]
      configure FDB entry
      configure MRU profile id and MRU value and bind it to ePort
      configure MRU command to DROP
      send untagged traffic with different MRU
      check counters and verify packets bigger than MRU value will be dropped.
      configure MRU command to FORWARD
      send untagged traffic with different MRU
      check counters and verify packets bigger than MRU value will be forwarded.
      configure MRU command to MIRROR to CPU
      configure CPU code exception to NET control
      send untagged traffic with different MRU
      check counters and verify packets bigger than MRU value will be received
       in CPU with correct code.
*/
UTF_TEST_CASE_MAC(tgfPortProfileMruCheck)
{
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/

    GT_U32      notAppFamilyBmp;
    GT_U32      profileId;

    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* Supported only by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    for (profileId = 0; profileId < 8; profileId++) {
        /* Set configuration */
        prvTgfPortProfileMruCheckConfigSet(profileId);

        /* Generate traffic */
        prvTgfPortProfileMruCheckTrafficGenerate();

        /* Restore configuration */
        prvTgfPortProfileMruCheckConfigRestore(profileId);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test TxQ ports:
    disable all ports
    test for every DQ[0..5] and for each txQ port[0..95]
      configure 2 VLAN entries (port[0] to send traffic, port[N] according to current DP/TxQ to recieve taffic)
      send one unknown UC packet to port[0]
      check counters - one packet received on port with configured TxQ.
    invalidate VLAN
    enable all ports
*/
UTF_TEST_CASE_MAC(tgfPortTxQPortsCheck)
{
    GT_U32  value;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_BOBCAT3_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
                                        | UTF_XCAT2_E | UTF_LION2_E
                                        | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E);

    /* initSerdesDefaults */
    if(appDemoDbEntryGet("initSerdesDefaults", &value) == GT_OK)
    {
        if(value)
        {
            SKIP_TEST_MAC
        }
        prvTgfPortTxMapPhy2TxQCheck();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test extended port mode (xcat3 only):

   Save MAC == PORT extended mode config
   Save MRU value
   Set MAC == PORT MRU value less then packet size
   Check the trafic - MAC == PORT  expect bad octets counter to be set
   Enable extended mode on port:  MAC != PORT
   Check the trafic - MAC != PORT expect NO bad packet octets counter to be set
   Restore MAC == PORT extended mode config
   Restore MRU value
*/
UTF_TEST_CASE_MAC(tgfPortExtendedMode)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_XCAT3_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
                                        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
                                        | UTF_XCAT2_E   | UTF_XCAT_E | UTF_LION_E | UTF_LION2_E
                                        | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E);

    prvTgfPortExtendedModeTest(25);

    prvTgfPortExtendedModeTest(27);
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test TailDrop table access under FWS:

check tabelse:
    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_MAX_QUEUE_LIMITS_E,
    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E,
    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E,
    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E,
    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E,
    CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E,

    by using cpssDxChPortTx4TcTailDropProfileSetUT

    1) WR/RD without traffic
    2) WR/RD with FWS traffic
    3) WR/RD without traffic


*/

UTF_TEST_CASE_MAC(tgfPortTxFwsTcTailDropProfile)
{
    GT_U32      notAppFamilyBmp;
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    if (IS_BOBK_DEV_CETUS_MAC(prvTgfDevNum))
    {
        /* the Cetus with TM has low BW and test became meaningless */
        PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;
    }

    #ifndef ASIC_SIMULATION
    /*prvPortTxFwsTcTailDropProfileWrRd();*/
    prvPortTxFullBandwidthTcTailDropProfileWrRd();
    #endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: DBA Queue Resources Allocation
Configuration.
1. Configure VLAN for port (0) and port(1)
2. Bind receive port (1) to Tail Drop Profile 7.
3. Enable DBA mode.
4. For traffic class 7 configure tail-drop profiles:
4.1. Set configuration for Tail Drop Profile 7
4.1.1. For DP 0 (green)  - 300 buffers, alpha = 4;
4.1.2. For DP 1 (yellow) - 200 buffers, alpha = 2;
4.1.3. For DP 2 (red)      - 100 buffers, alpha = 0.5;
4.2. Set configuration for Tail Drop Profile 0
4.2.1. For tail-drop profile 0 set alpha = 0, 0 buffers for all DP
4.2.2. For tail-drop profile 7 set alpha = 0, 0 buffers for all DP
5. For every traffic class, except 7, range  configure tail-drop profiles:
5.1. For tail-drop profile 0 set alpha=0 for all DP
5.2. For tail-drop profile 7 set alpha=0 for all DP
6. Set amount of available for DBA buffers to 1000.

Generate traffic
1. Clear MAC counters.
2. Go over all Drop Precedences (green/yellow/red).
2.1. Get expected amount of available buffers (expectedValue).
2.2. Configure COS profile 1023 with Drop Precedence and Traffic Class (7) of ingress frames. Assign the default COS profile 1023 to receive port 1.
2.3. Block TxQ (7) for receive port (1).
2.4. Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
2.5. Release TxQ (7) for receive port (1).
2.6. Read MAC counters.

Expected
Amount of sent frames will be in range: [ expectedValue-10 .. expectedValue+10 ].
*/
UTF_TEST_CASE_MAC(tgfPortTxTailDropDbaQueueResources)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    tgfPortTxTailDropDbaQueueResourcesConfigure(GT_TRUE);
    tgfPortTxTailDropDbaQueueResourcesTrafficGenerate();
    tgfPortTxTailDropDbaQueueResourcesConfigure(GT_FALSE);
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: DBA Port Resources Allocation
Configuration.
1. Configure VLAN for port (0) and port(1)
2. Bind receive port (1) to Tail Drop Profile 7.
3. Enable DBA mode.
4. Set alpha = 0 for Tail Drop {TC,DP} configuration for Profiles 0 and 7.
5. Set alpha = 0 for Tail Drop per port configuration for Profile (7).
6. Set amount of available for DBA buffers to 1000.

Generate traffic
1. Clear MAC counters.
2. Go over all alphas.
2.1. Set portAlpha for receive port (1).
2.2. Get expected amount of available buffers (expectedValue).
2.3. Block TxQ (7) for receive port (1).
2.4. Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
2.5. Release TxQ (7) for receive port (1).
2.6. Read MAC counters.

Expected
Amount of sent frames will be in range: [ expectedValue-10 .. expectedValue+10 ].
*/

UTF_TEST_CASE_MAC(tgfPortTxTailDropDbaPortResources)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#ifndef ASIC_SIMULATION
    tgfPortTxTailDropDbaPortResourcesConfigure(GT_TRUE);
    tgfPortTxTailDropDbaPortResourcesTrafficGenerate();
    tgfPortTxTailDropDbaPortResourcesConfigure(GT_FALSE);
#endif
}

UTF_TEST_CASE_MAC(tgfPortFwdToLbForwardSingleTarget)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* xCat3x excluded because of problems with 'Forwarding to Loopback' feature.
      1) xCat3x CCFC and "cpssDxChPortLoopback..." interfere with each other
         because they both use the same fields in the table
         CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E
      2) FORWARD/FROM_CPU/TO_ANALYZER packets goto 88e1690 as 4B FROM_CPU DSA
         tagged. There are no such configuration for TO_CPU packet type.
         So TO_CPU packets forwarded to a loopback port which is remote port,
         comes to 88e1690 4B TO_CPU DSA tagged and are filtered. */
    notAppFamilyBmp |= UTF_AC3X_E;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    tgfPortFwdToLbForwardSingleTargetTest();
}

UTF_TEST_CASE_MAC(tgfPortFwdToLbForwardMultiTarget)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* xCat3x excluded because of problems with 'Forwarding to Loopback' feature.
      1) xCat3x CCFC and "cpssDxChPortLoopback..." interfere with each other
         because they both use the same fields in the table
         CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E
      2) FORWARD/FROM_CPU/TO_ANALYZER packets goto 88e1690 as 4B FROM_CPU DSA
         tagged. There are no such configuration for TO_CPU packet type.
         So TO_CPU packets forwarded to a loopback port which is remote port,
         comes to 88e1690 4B TO_CPU DSA tagged and are filtered. */
    notAppFamilyBmp |= UTF_AC3X_E;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    tgfPortFwdToLbForwardMultiTargetTest();
}

UTF_TEST_CASE_MAC(tgfPortFwdToLbToAnalyzer)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* xCat3x excluded because of problems with 'Forwarding to Loopback' feature.
     * 1) xCat3x CCFC and "cpssDxChPortLoopback..." APIs interfere with each other
     *    because they both use the same fields in the table
     *    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E
     * 2) FORWARD/FROM_CPU/TO_ANALYZER go to 88e1690 as 4B FROM_CPU DSA tagged.
          But there are not such configuration for TO_CPU packet type.
          So TO_CPU packets forwarded to loopback port which is remote port,
     *    comes to 88e1690 as 4B TO_CPU DSA tagged and are filtered there.*/
    notAppFamilyBmp |= UTF_AC3X_E;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    tgfPortFwdToLbToAnalyzerTest();
}

UTF_TEST_CASE_MAC(tgfPortFwdToLbToCpu)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

     /* xCat3x excluded because of problems with 'Forwarding to Loopback' feature.
      1) xCat3x CCFC and "cpssDxChPortLoopback..." interfere with each other
         because they both use the same fields in the table
         CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E
      2) FORWARD/FROM_CPU/TO_ANALYZER packets goto 88e1690 as 4B FROM_CPU DSA
         tagged. There are no such configuration for TO_CPU packet type.
         So TO_CPU packets forwarded to a loopback port which is remote port,
         comes to 88e1690 4B TO_CPU DSA tagged and are filtered. */
    notAppFamilyBmp |= UTF_AC3X_E;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    {
        GT_U32      boardIdx;
        GT_U32      boardRevId;
        GT_U32      reloadEeprom;

        cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

        /* skip multidevice boards */
        if((boardIdx == 30) /* BC2 x 6 */ ||
           (boardIdx == 31) /* Lion2 + BC2 */)
        {
            SKIP_TEST_MAC;
        }
    }

    tgfPortFwdToLbToCpuTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Check filtered multicast counters
Configuration.
1. Configure VLAN 5 for 4 ports
2. Set all of the ports to drop unregistered multicast packets

Generate traffic
1. Clear counters.
2. Send unregistered multicast packet to one of the ports (send port)
3. Check that filtered multicast counter incremented
4. Check MAC counters (no traffic egressed from receiving port)
5. Enable unregisted multicast packets on one of the receiving ports
6. Send unregistered multicast packet to one of the ports (send port)
7. Check that filtered multicast counter not incremented
8. Check MAC counters (one of the receiving ports has incremented "sent" counters)
*/
UTF_TEST_CASE_MAC(prvTgfPortStatMcFilterDropPkts)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp |= (UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_BOBCAT3_E |
                        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E |
                        UTF_ALDRIN2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfPortStatMcFilterDropPktsConfigure(GT_TRUE);
    prvTgfPortStatMcFilterDropPktsGenerate();
    prvTgfPortStatMcFilterDropPktsConfigure(GT_FALSE);
}

/*
 * Configuration of tgfPort suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfPort)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibToCpuTrafficCaptureCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibCpuTrafficCaptureCntr)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibBroadcastCaptureCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibUnicastCaptureCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibMulticastCaptureCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibOversizeCaptureCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibHistogramCaptureCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortMacMibClearOnReadCaptureCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxResourceHistogramCntr)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortRemotePortsMacCountersByPacket)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxSchedulerSp)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortIfModeSpeed)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortFlowControlPacketsCounter)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortProfileMruCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxQPortsCheck)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortExtendedMode)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxFwsTcTailDropProfile)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortFwdToLbForwardSingleTarget)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortFwdToLbForwardMultiTarget)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortFwdToLbToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortFwdToLbToCpu)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxTailDropDbaQueueResources)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxTailDropDbaPortResources)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPortStatMcFilterDropPkts)
UTF_SUIT_END_TESTS_MAC(tgfPort)
