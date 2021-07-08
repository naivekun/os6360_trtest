/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxTgfPortTxTailDropUT.c
*
* @brief Enhanced UTs for CPSS PX Port Tx Tail Drop APIs
*
* @version   1
********************************************************************************
*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utfTraffic/common/cpssPxTgfCommon.h>
#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropVariousPorts.h>
#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropOnePort.h>
#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropOnePortSharing.h>
#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropSharedPools.h>
#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropDbaQueueResources.h>
#include <utfTraffic/port/prvCpssPxTgfPortTxTailDropDbaPortResources.h>


/*----------------------------------------------------------------------------*/
/*
    Test 1: DPs different limits - same profile, various ports.

    Goals: Verify behavior according to different DPs limits.

    Configuration:
        1. Configure Packet Type entry 0 - all ingress frames with MAC DA
           00:01:02:03:AA:01 will be classified as Packet Type 0.
        2. Set index of Destination Port Map Table entry for Packet Type 0.
        3. Bind egress ports 1,2,3 to Tail Drop Profile 7.
        4. Disable DBA mode.
        5. Set buffers/descriptors limits per Tail Drop Profile 7 and
           Traffic Class 7:
            - for Drop Precedence 0 (green)  - 1000 buffers/descriptors;
            - for Drop Precedence 1 (yellow) -  500 buffers/descriptors;
            - for Drop Precedence 2 (red)    -  200 buffers/descriptors.
        6. Set port limits for Tail Drop Profile 7 - 1700 buffers/descriptors.

    Traffic:
        1. Go over all Drop Precedence (green/yellow/red).
        1.1. Clear MAC counters.
        1.2. Set Drop Precedence and Traffic Class (7) of ingress frames for
             Packet Type (0).
        1.3. Set entry of Destination Port Map Table - disable forwarding to
             all ports except receive port (1/2/3)
        1.4. Block TxQ 7 for receive port (1/2/3).
        1.5. Transmit 1500 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
        1.6. Release TxQ 7 for receive port (1/2/3).
        1.7. Check MAC counters.

    Expected:
        Tx counter on receive port 1/2/3 should be 1000/500/200 frames
        respectively.
*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxTgfPortTxTailDropVariousPorts)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    prvCpssPxTgfPortTxTailDropVariousPortsConfigurationSet();

    prvCpssPxTgfPortTxTailDropVariousPortsTrafficGenerate();

    prvCpssPxTgfPortTxTailDropVariousPortsConfigurationRestore();

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 2: DPs different limits - one port.

    Goals: Verify behavior according to different DPs limits.

    Configuration:
        1. Configure Packet Type entry (0) - all ingress frames with MAC DA
           00:01:02:03:AA:01 will be classified as Packet Type 0.
        2. Set index of Destination Port Map Table entry for Packet Type 0.
        3. Set entry of Destination Port Map Table - disable forwarding to all
           ports except receive port (1).
        4. Bind egress port (1) to Tail Drop Profile 7.
        5. Disable DBA mode.
        6. Set buffers/descriptors limits per Tail Drop Profile (7) and
           Traffic Class (7):
            - for Drop Precedence 0 (green)  - 1000 buffers/descriptors;
            - for Drop Precedence 1 (yellow) -  500 buffers/descriptors;
            - for Drop Precedence 2 (red)    -  200 buffers/descriptors;
        7. Set port limits for Tail Drop Profile 7 - 1700 buffers/descriptors.

    Traffic and expected results:
        1. Clear MAC counters.
        2. Block TxQ (7) for receive port (1).
        3. Go over all Drop Precedence (green/yellow/red).
        3.1. Set Drop Precedence and Traffic Class (7) of ingress frames for
             Packet Type (0).
        3.2. Transmit 1100 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
        3.3. Check count of global allocated buffers and descriptors - expected
             values 200 / 500 / 1000.
        4. Release TxQ (7) for receive port (1).
        5. Check MAC counters - expected Tx counter on port 1 is 1000 frames.
*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxTgfPortTxTailDropOnePort)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    prvCpssPxTgfPortTxTailDropOnePortConfigurationSet();

    prvCpssPxTgfPortTxTailDropOnePortTrafficGenerate();

    prvCpssPxTgfPortTxTailDropOnePortConfigurationRestore();

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 3: DPs different limits with sharing - one port.

    Goals: Verify behavior according to different DPs limits with sharing.

    Configuration:
        1. Configure Packet Type entry (0) - all ingress frames with MAC DA
           00:01:02:03:AA:01 will be classified as Packet Type 0.
        2. Set index of Destination Port Map Table entry for Packet Type 0.
        3. Set entry of Destination Port Map Table - disable forwarding to all
           ports except receive port (1).
        4. Bind egress port (1) to Tail Drop Profile 6.
        5. Disable DBA mode.
        6. Set buffers/descriptors limits per Tail Drop Profile (6) and
           Traffic Class (4):
            - for Drop Precedence 0 (green)  - 300 buffers/descriptors;
            - for Drop Precedence 1 (yellow) - 200 buffers/descriptors;
            - for Drop Precedence 2 (red)    - 100 buffers/descriptors;
        7. Set port limits for Tail Drop Profile 6 - 400 buffers/descriptors.
        8. Set limits for Shared Pool 2 - 70 buffers/descriptors.

    Traffic and expected results:
        1. Go over all Shared DP modes (disable/dp0/dp0-dp1/all).
        1.1. Set Shared DP mode and Pool Number (2) for test Tail Drop Profile
             and Traffic Class.
        1.2. Clear MAC counter for receive port (1).
        1.3. Go over all Drop Precedence.
        1.3.1. Block TxQ (4) for receive port (1).
        1.3.2. Set Drop Precedence (green/yellow/red) and Traffic Class (4) of
               ingress frames for Packet Type (0).
        1.3.3. Transmit 400 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
        1.3.4. Check global buffers/descriptors allocation.
        1.3.5. Release TxQ (4) for receive port (1).
        1.4. Check MAC counter on receive port (1).
*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxTgfPortTxTailDropOnePortSharing)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationSet();

    prvCpssPxTgfPortTxTailDropOnePortSharingTrafficGenerate();

    prvCpssPxTgfPortTxTailDropOnePortSharingConfigurationRestore();

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 4: 8 shared pools per 8 TC's

    Goals: Verify each traffic classes TC uses it's matching shared pool.

    Configuration:
        1. Go over all Traffic Classes (0..7).
        1.1. Configure Packet Type entry (2..9) - ingress frames will be
             classified by MAC DA.
             Note: use packet types in range ingress_port...(ingress_port+7)
        1.2. Configure CoS format entry - set Traffic Class (0..7) of ingress
             frames for current Packet Type (2..9).
        1.3. Set index of Destination Port Map Table Entry for Packet Type.
        2. Set entry of Destination Port Map Table - disable forwarding to all
           ports except receive port (1).
        3. Disable DBA mode.
        4. Bind egress port (1) to Tail Drop Profile 7.
        5. Set port limits for Tail Drop Profile 7 - 300 buffers/descriptors.
        6. Go over all Traffic Classes (0..7).
        6.1. Set buffers/descriptors limits per Tail Drop Profile (7) and
             Traffic Class (0..7): 200 buffers/descriptors for DP0/DP1/DP2.
        6.2. Bind Traffic Class (0..7) to Shared Pool (0..7).
        6.3. Set buffers/descriptors limit per Shared Pool: 70 + (0..7).

    Generate traffic:
        1. Go over all Traffic Class (0..7).
        1.1. Set MAC DA for transmitted frames
             (00:01:02:03:AA:00 .. 00:01:02:03:AA:07)
        1.2. Clear MAC counters for receive port.
        1.3. Block TxQ (0..7) for receive port (1).
        1.4. Transmit 300 frames to send port (0).
        1.5. Release TxQ (0..7) for receive port (1).
        1.6. Read MAC counters for receive port (1).
             Expected value for Tx counters: 270 + (0..7) frames.
*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxTgfPortTxTailDropSharedPools)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationSet();

    prvCpssPxTgfPortTxTailDropSharedPoolsTrafficGenerate();

    prvCpssPxTgfPortTxTailDropSharedPoolsConfigurationRestore();

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 5: DBA Queue Resources Allocation

    Goals: Verify dynamic buffers allocation per Traffic Class and
           Drop Precedence.

    Configuration:
        1. Configure Packet Type entry (0) - all ingress frames with MAC DA
           00:01:02:03:AA:01 will be classified as Packet Type 0.
        2. Set index of Destination Port Map Table entry for Packet Type 0.
        3. Set entry of Destination Port Map Table - disable forwarding to all
           ports exclude receive port (1).
        4. Bind egress port (1) to Tail Drop Profile 7.
        5. Enable DBA mode.
        6. Set configuration for Tail Drop Profile (7) and Traffic Class (7):
            - for Drop Precedence 0 (green)  - 300 buffers; Alpha 4;
            - for Drop Precedence 1 (yellow) - 200 buffers; Alpha 2;
            - for Drop Precedence 2 (red)    - 100 buffers; Alpha 0.5;
        7. Set Alphas for default Tail Drop Profile (0) (all TCs) and all TCs
           exclude TC 7 for Tail Drop Profile 7 to Alpha 0.
        8. Set amount of available for DBA buffers to 1000.

    Generate traffic:
        1. Clear MAC counters.
        2. Go over all Drop Precedences (green/yellow/red).
        2.1. Get expected amount of available buffers (expectedValue).
        2.2. Set Drop Precedence and Traffic Class (7) of ingress frames for
             Packet Type (0).
        2.3. Block TxQ (7) for receive port (1).
        2.4. Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
        2.5. Release TxQ (7) for receive port (1).
        2.6. Read MAC counters.

    Expected:
        Amount of sent frames will be in range:
        [ expectedValue-10 .. expectedValue+10 ].
*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(prvCpssPxTgfPortTxTailDropDbaQueueResources)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    prvCpssPxTgfPortTxTailDropDbaQueueConfigurationSet();

    prvCpssPxTgfPortTxTailDropDbaQueueTrafficGenerate();

    prvCpssPxTgfPortTxTailDropDbaQueueConfigurationRestore();

#endif /* ASIC_SIMULATION */
}

/*----------------------------------------------------------------------------*/
/*
    Test 6: DBA Port Resources Allocation

    Goals: Verify Tail Drop dynamic buffers allocation per port.

    Configuration:
        1. Configure Packet Type entry (0) - all ingress frames with MAC DA
           00:01:02:03:AA:01 will be classified as Packet Type 0.
        2. Set index of Destination Port Map Table entry for Packet Type 0.
        3. Set Traffic Class (7) of ingress frames for Packet Type (0).
        4. Set entry of Destination Port Map Table - disable forwarding to all
           ports exclude receive port (1).
        5. Bind egress port (1) to Tail Drop Profile 7.
        6. Enable DBA mode.
        7. Set Alpha0 for Tail Drop {TC,DP} configuration for Profiles 0 and 7.
        8. Set Alpha0 for Tail Drop per port configuration for Profile (7).
        9. Set amount of available for DBA buffers to 1000.

    Traffic:
        1. Clear MAC counters.
        2. Go over all Alphas.
        2.1. Set portAlpha for receive port.
        2.2. Get expected amount of available buffers (expectedValue).
        2.3. Block TxQ (7) for receive port (1).
        2.4. Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to port 0.
        2.5. Release TxQ (7) for receive port (1).
        2.6. Read MAC counters.

    Expected:
        Amount of sent frames will be in range:
        [ expectedValue-10 .. expectedValue+10 ].
*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(prvCpssPxTgfPortTxTailDropDbaPortResources)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    prvCpssPxTgfPortTxTailDropDbaPortConfigurationSet();

    prvCpssPxTgfPortTxTailDropDbaPortTrafficGenerate();

    prvCpssPxTgfPortTxTailDropDbaPortConfigurationRestore();

#endif /* ASIC_SIMULATION */
}

UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfPortTxTailDrop)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPortTxTailDropVariousPorts)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPortTxTailDropOnePort)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPortTxTailDropOnePortSharing)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxTgfPortTxTailDropSharedPools)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssPxTgfPortTxTailDropDbaQueueResources)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssPxTgfPortTxTailDropDbaPortResources)
UTF_SUIT_END_TESTS_MAC(cpssPxTgfPortTxTailDrop)

