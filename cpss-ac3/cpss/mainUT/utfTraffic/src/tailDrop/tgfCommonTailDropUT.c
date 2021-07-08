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
* @file tgfCommonTailDropUT.c
*
* @brief Enhanced UTs for CPSS Tail Drop
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <tailDrop/prvTgfTailDrop.h>
#include <tailDrop/prvTgfTailDropTxqUtil.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>


/*----------------------------------------------------------------------------*/
/*
    Test 1: DPs different limits - same profile, various ports.

    Goals: Verify behavior according to different DPs limits.

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      3 FDB, 3 traffic classes, 3 PCLs=set tc
      1 Drop profile 8: Dp[G,Y,R]=[1000,500,200] packets
      ===> Bind ports 0,8,23 to profile 8 <===

    Traffic:
      Block TxQ=7 for ports 0,8,23
      1. Send 1200 packets from port 18 to MAC DA0
      2. Send 1200 packets from port 18 to MAC DA1
      3. Send 1200 packets from port 18 to MAC DA2
      Release TxQ=7 for ports 0,8,23

    Expected results:
        - Verify ports 0,8,23 received 1000,500.200 packets respectively
*/
UTF_TEST_CASE_MAC(tgfTailDropVariousPorts)
{
   PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* Set 3 FDB, 3 QOS and 3 PCL */
    prvTgfTailDropConfigurationSet(GT_TRUE, 3, CPSS_DP_GREEN_E);

    /* Set Tail Drop Profile 8 for ports 0,8,23 */
    prvTgfTailDropProfileConfigurationSet(GT_TRUE);

    /* Generate traffic */
    prvTgfTailDropVariousPortsTrafficGenerate(1200);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(3);
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 2: DPs different limits - one port.

    Goals: Verify behavior according to different DPs limits with sharing.

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      3 FDB, 3 Tc, 3 PCL=set tc
      1 Drop profile 8: Dp[G,Y,R]=[1000,500,200] packets
      ===> Bind port 8 to profile 8 <===

    Traffic and expected results:
      Block TxQ=7 for port 8
      1. Send 1200 packets from port 18 to MAC DA2
        - Verify allocated buffers and descriptors equal 200
      2. Send 1200 packets from port 18 to MAC DA1
        - Verify allocated buffers and descriptors equal 500
      3. Send 1200 packets from port 18 to MAC DA0
        - Verify allocated buffers and descriptors equal 1000
      Release TxQ=7 for port 8
        - Verify port 8 received 1000 packets
*/
UTF_TEST_CASE_MAC(tgfTailDropOnePort)
{
   PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* Set 3 FDB, 3 QOS and 3 PCL */
    prvTgfTailDropConfigurationSet(GT_FALSE, 3, CPSS_DP_GREEN_E);

    /* Set Tail Drop Profile 8 for port 8 */
    prvTgfTailDropProfileConfigurationSet(GT_FALSE);

    /* Generate traffic for all FDB entries (fdbIdxMin = 3) */
    prvTgfTailDropOnePortTrafficGenerate(3, 1200);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(3);
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 3: DPs different limits with sharing - one port.

    Goals: Verify behavior according to different DPs limits.

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      3 FDB, 3 Tc, 3 PCL=set tc
      1 Drop profile 8: Dp[G,Y,R]=[1000,500,200] packets
      ===> Bind port 8 to profile 8 <===

    Traffic and expected results:
      ===> TestCase 0: Enable sharing only for DP0 <===
         Bind Shared Pool 2 to profile 8; Tc 7
         Set Shared Pool Limits to 500
      Block TxQ=7 for port 8
      1. Send 2000 packets from port 18 to MAC DA2
        - Verify allocated buffers and descriptors equal 200
      2. Send 2000 packets from port 18 to MAC DA1
        - Verify allocated buffers and descriptors equal 500
      3. Send 2000 packets from port 18 to MAC DA0
        - Verify allocated buffers and descriptors equal 1501
      Release TxQ=7 for port 8
        - Verify port 8 received 1501 packets

      ===> TestCase 1: Enable sharing for DP0 & DP1 <===
      Block TxQ=7 for port 8
      1. Send 2000 packets from port 18 to MAC DA2
        - Verify allocated buffers and descriptors equal 200
      2. Send 2000 packets from port 18 to MAC DA1
        - Verify allocated buffers and descriptors equal 1501
      Release TxQ=7 for port 8
        - Verify port 8 received 1501 packets

      ===> TestCase 2: Enable sharing for all DPs <===
      Block TxQ=7 for port 8
      1. Send 2000 packets from port 18 to MAC DA2
        - Verify allocated buffers and descriptors equal 1501
      Release TxQ=7 for port 8
        - Verify port 8 received 1501 packets
*/
UTF_TEST_CASE_MAC(tgfTailDropOnePortSharing)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* if not Aldrin2 the function does nothing */
    appDemoDxAldrin2TailDropDbaEnableSet(prvTgfDevNum, GT_FALSE);

    /* Set 3 FDB, 3 QOS and 3 PCL */
    prvTgfTailDropConfigurationSet(GT_FALSE, 3, CPSS_DP_GREEN_E);

    /* Set Tail Drop Profile 8 for port 8 */
    prvTgfTailDropProfileConfigurationSet(GT_FALSE);

    /* Set MODE_DP0 */
    PRV_UTF_LOG0_MAC("\n================== SHARED_DP_MODE_DP0 ===================\n");
    prvTgfTailDropSharingConfigurationSet(PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_E);

    /* Generate traffic for the fdbIdxMin = 0 */
    prvTgfTailDropOnePortTrafficGenerate(0, 2000);

    /* Set MODE_DP0_DP1 */
    PRV_UTF_LOG0_MAC("\n================ SHARED_DP_MODE_DP0_DP1 =================\n");
    prvTgfTailDropSharingConfigurationSet(PRV_TGF_PORT_TX_SHARED_DP_MODE_DP0_DP1_E);

    /* Generate traffic for the fdbIdxMin = 1 */
    prvTgfTailDropOnePortTrafficGenerate(1, 2000);

    /* Set MODE_All */
    PRV_UTF_LOG0_MAC("\n================== SHARED_DP_MODE_ALL ===================\n");
    prvTgfTailDropSharingConfigurationSet(PRV_TGF_PORT_TX_SHARED_DP_MODE_ALL_E);

    /* Generate traffic for the fdbIdxMin = 2 */
    prvTgfTailDropOnePortTrafficGenerate(2, 2000);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(3);

    /* if not Aldrin2 the function does nothing */
    appDemoDxAldrin2TailDropDbaEnableRestore(prvTgfDevNum);

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 4: 8 shared pools per 8 Tc's

    Goals: Verify each traffic classes Tc uses it's matching shared pool.

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      8 FDB, 8 Tc, 8 PCL=set tc
      1 Drop profile 7: Dp[G,Y,R]=[200,0,0] packets
      8 shared pool profiles, 8 pool limits for each Tc
      ===> Bind port 8 to profile 7, Tc0..7 <===

    Traffic and expected results:
      for (i=0..7){
        Block TxQ[i] for port 8
          Send 2000 packets from port 18 to MAC DA[i]
          - Verify allocated buffers and descriptors
        Release TxQ[i] for port 8
          - Verify port 8 counters
      }
*/
UTF_TEST_CASE_MAC(tgfTailDropSharedPools)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* if not Aldrin2 the function does nothing */
    appDemoDxAldrin2TailDropDbaEnableSet(prvTgfDevNum, GT_FALSE);

    /* Set 8 FDB, 8 QOS and 8 PCL */
    prvTgfTailDropConfigurationSet(GT_FALSE, 8, CPSS_DP_GREEN_E);

    /* Set 8 Shared Pools */
    prvTgfTailDropSharedPoolsConfigurationSet();

    /* Generate traffic */
    prvTgfTailDropSharedPoolsTrafficGenerate(2000);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(8);

    /* if not Aldrin2 the function does nothing */
    appDemoDxAldrin2TailDropDbaEnableRestore(prvTgfDevNum);
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 5: Weighted Random Tail-drop - {Queue,DP} Limit.

    Goals: Verify a certain percentage of packets is dropped
           when WRTD is enabled using {Queue,Dp} Limit.

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      1 FDB, 1 Tc with Dp=Green, 1 PCL
      1 Drop profile 1: Dp[G,Y,R]=[1536,0,0] packets
      ===> Bind port 8 to profile 1 <===
      Set Egress counters on port 8 with Tc=4, Dp=Green

    Traffic and expected results:
      1. Disable WRTD
      Block TxQ=4 for port 8
        Send 1600 packets
        - Verify allocated buffers and descriptors equal 1536
      Release TxQ=4 for port 8
        - Verify port 8 received 1536 packets

      2. Enable WRTD
      Block TxQ=4 for port 8
        Send 1600 packets
        - Verify allocated buffers and descriptors in range 1536..1600
      Release TxQ=4 for port 8
        - Verify port 8 Eth counters in range 1536..1600 packets
        - Verify port 8 Egress counters in range 1536..1600 packets
*/
UTF_TEST_CASE_MAC(tgfTailDropWrtdQueueDp)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;
{
    PRV_TGF_DROP_CONFIG_SET_ENT configSetIdx   = PRV_TGF_DROP_CONFIG_SET_0_E;
    CPSS_DP_LEVEL_ENT           dropPrecedence = CPSS_DP_GREEN_E;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, (GT_U32)(UTF_BOBCAT3_E | UTF_FALCON_E)))
    {
        /* BC3: feature does not work on DP0 (see FE-1298799) - use DP1 */
        configSetIdx   = PRV_TGF_DROP_CONFIG_SET_2_E;
        dropPrecedence = CPSS_DP_YELLOW_E;
    }

    /* Set 1 FDB, 1 QOS and 1 PCL */
    prvTgfTailDropConfigurationSet(GT_FALSE, 1, dropPrecedence);

    /* Set Tail Drop Profile 8 for port 8 */
    prvTgfTailDropWrtdConfigurationSet(configSetIdx);

    /* Generate traffic with Disable WRTD */
    prvTgfTailDropWrtdTrafficGenerate(GT_FALSE, 1600, 1536, configSetIdx);

    /* Generate traffic with Enable WRTD */
    prvTgfTailDropWrtdTrafficGenerate(GT_TRUE, 1600, 1536, configSetIdx);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(1);
}
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 6: Weighted random Tail drop - Port limit.

    Goals: Verify a certain percentage of packets is dropped
           when WRTD is enabled using Port Limit.

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      1 FDB, 1 Tc with Dp=Yellow, 1 PCL
      1 Drop profile 5: Dp[G,Y,R]=[1900,1750,1000] packets
      1 Drop profile on port 8: Dp=256 packets
      ===> Bind port 8 to profile 5 <===
      Set Egress counters on port 8 with Tc=7, Dp=Yellow

    Traffic and expected results:
      1. Disable WRTD
      Block TxQ=7 for port 8
        Send 600 packets
        - Verify allocated buffers and descriptors equal 257
      Release TxQ=7 for port 8
        - Verify port 8 received 257 packets

      2. Enable WRTD
      Block TxQ=7 for port 8
        Send 600 packets
        - Verify allocated buffers and descriptors in range 257..512
      Release TxQ=7 for port 8
        - Verify port 8 Eth counters in range 257..512 packets
        - Verify port 8 Egress counters in range 257..512 packets
*/
UTF_TEST_CASE_MAC(tgfTailDropWrtdPort)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* Set 1 FDB, 1 QOS and 1 PCL */
    prvTgfTailDropConfigurationSet(GT_FALSE, 1, CPSS_DP_YELLOW_E);

    /* Set Tail Drop Profile 8 for port 8 */
    prvTgfTailDropWrtdConfigurationSet(PRV_TGF_DROP_CONFIG_SET_1_E);

    /* Generate traffic with Disable WRTD */
    prvTgfTailDropWrtdTrafficGenerate(GT_FALSE, 600, 257, PRV_TGF_DROP_CONFIG_SET_1_E);

    /* Generate traffic with Enable WRTD */
    prvTgfTailDropWrtdTrafficGenerate(GT_TRUE, 600, 257, PRV_TGF_DROP_CONFIG_SET_1_E);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(1);
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 7: Tail drop - Limiting for MC packets.

    Goals: - Verify all packets that exceed Multicast Limit are dropped;
           - Verify non multicast packets are not affected by limit;

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      1 FDB
      1 Drop profile 3: Dp[G,Y,R]=[1900,0,0] packets
      ===> Bind port 18 to profile 3 <===

    Traffic and expected results:
      Block TxQ=0 on port 18
      1. Send 2000 packets from port  0 to MAC DA0 - unknown UC
      2. Send 1000 packets from port 18 to MAC DA1 - known UC
      Release TxQ=0 on port 18

    Expected results:
        - Verify Mc buffers and descriptors equal 601
        - Verify Global buffers and descriptors equal 1601
        - Verify ports 0,8,18,23 received 0,601,1601,601 packets respectively
*/
UTF_TEST_CASE_MAC(tgfTailDropMc)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* Set configuration */
    prvTgfTailDropMcConfigurationSet();

    /* Generate traffic */
    prvTgfTailDropMcTrafficGenerate(2000, 1000, 601, 1601);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(1);
    tgfTailDropSeparateUcMcCountPerQueueConfigurationRestore();
#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 8: Tail drop - Separate UC and MC counting per queue
*/
/* AUTODOC: Check seaprate/shared Unicast/Multicast buffer counting

    Configuration:
      1 Vlan, 4 ports; HOL system mode enable
      1 FDB
      1 Drop profile 3: Dp[G,Y,R]=[1900,0,0] packets
      ===> Bind port 18 to profile 3 <===

    Add. Configuration:
      profile 3 is configured for
        - shared counting
        - UC limit = 10
        - MC limit = 10

    Traffic and expected results:
      Block TxQ=0 on port 18
      1. Send 10 packets from port  0 to MAC DA0 - unknown UC
      2. Send 10 packets from port 18 to MAC DA1 - known UC
      Release TxQ=0 on port 18

    Expected results:
        - Verify ports 0,8,18,23 received 0,10,10,10 packets respectively

    Add. Configuration:
      profile 3 is configured for
        - separate counting
        - UC limit = 10
        - MC limit = 10

    Traffic and expected results:
      Block TxQ=0 on port 18
      1. Send 10 packets from port  0 to MAC DA0 - unknown UC
      2. Send 10 packets from port 18 to MAC DA1 - known UC
      Release TxQ=0 on port 18

    Expected results:
        - Verify ports 0,8,18,23 received 0,10,20,10 packets respectively
*/
UTF_TEST_CASE_MAC(tgfTailDropSeparateUcMcCountPerQueue)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA2_E | UTF_PUMA3_E);
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* tail drop in TxQ is not functional for systems in traffic manager */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    /* Set configuration */
    tgfTailDropSeparateUcMcCountPerQueueConfigurationSet(GT_TRUE, GT_FALSE);

    /* Generate traffic */
    tgfTailDropSeparateUcMcCountPerQueueTrafficGenerate(10, 10, 10);

    /* Set configuration */
    tgfTailDropSeparateUcMcCountPerQueueConfigurationSet(GT_FALSE, GT_TRUE);

    /* Generate traffic */
    tgfTailDropSeparateUcMcCountPerQueueTrafficGenerate(10, 10, 20);

    /* Restore configuration */
    prvTgfTailDropConfigurationRestore(1);
    tgfTailDropSeparateUcMcCountPerQueueConfigurationRestore();

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: TxQ utilization statistics
1. Enable statistics
2. Configure VLAN for port (0) and port(1)
3. Assign QoS profile 1023 to ALL packets on sending ports.
4. For every traffic class tc
4.1. Configure COS profile 1023 with Traffic Class of ingress frames. Assign the default COS profile 1023 to receive port 1.
4.2. Block TxQ for receive port (1).
4.3. Transmit 100 frames to port 0.
4.4. Release TxQ for receive port (1).
4.5. Read statistics. Expect some peaks detected.
*/
UTF_TEST_CASE_MAC(prvTgfTailDropTxqUtil)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E);

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#ifndef ASIC_SIMULATION
    prvTgfTailDropTxqUtilConfigure(GT_TRUE);
    prvTgfTailDropTxqUtilTrafficGenerate();
    prvTgfTailDropTxqUtilConfigure(GT_FALSE);
#endif
}

/*
 * Configuration of tgfTailDrop suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfTailDrop)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropVariousPorts)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropOnePort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropOnePortSharing)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropSharedPools)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropWrtdQueueDp)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropWrtdPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropMc)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTailDropSeparateUcMcCountPerQueue)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfTailDropTxqUtil)
UTF_SUIT_END_TESTS_MAC(tgfTailDrop)


