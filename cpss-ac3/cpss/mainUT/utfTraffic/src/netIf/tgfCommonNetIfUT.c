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
* @file tgfCommonNetIfUT.c
*
* @brief Enhanced UTs for CPSS network interface
*
* @version   12
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfNetIfGen.h>

#include <netIf/prvTgfNetIfTxSdmaGeneratorLinkList.h>
#include <netIf/prvTgfNetIfTxSdmaGeneratorQEnable.h>
#include <netIf/prvTgfNetIfTxSdmaGeneratorTwoQueues.h>
#include <netIf/prvTgfNetIfTxSdmaGeneratorFlowInterval.h>
#include <netIf/prvTgfNetIfTxSdmaGeneratorGlobalThroughput.h>
#include <netIf/prvTgfNetIfTxSdmaGeneratorBurstTx.h>
#include <netIf/prvTgfNetIfTxSdmaGeneratorTransitionState.h>
#include <netIf/prvTgfNetIfTxSdmaGeneratorErrorReport.h>
#include <netIf/prvTgfNetIfSdmaPerformance.h>
#include <netIf/prvTgfNetIfTxSdmaEvents.h>
#include <netIf/prvTgfNetIfFromCpuToVidxExcludedPort.h>
#include <netIf/prvTgfNetIfSdmaRxQEnable.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Link List manipulations:
   Creating a circular chain and performing various linked list packet
   related operations - add, update & remove.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorLinkList)
{
    GT_U32 notAppFamilyBmp;
    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfTxSdmaGeneratorLinkListTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Queue implicit enabling and disabling
   Queue "remembers" user last enable or disable request.
   When enabling an empty queue the queue should not be "actually" move into
   enable state, but should be enabled (implicitly) only after the first packet
   addition to its list.
   Similarly, when the last packet from the list is removed and the queue was
   not explicitly disabled before it must be implicitly disabled.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorQEnable)
{
    GT_U32 notAppFamilyBmp;
    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfTxSdmaGeneratorQEnableTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Two Queues manipulations:
   Distinguish between chains of two different Tx queues which co-exist.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorTwoQueues)
{
    GT_U32 notAppFamilyBmp;
    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfTxSdmaGeneratorTwoQueuesTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Rate adaptation for flow interval mode:
   Generator rate configured in flow interval mode is kept during packets
   addition and removal.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorFlowInterval)
{
    GT_U32 notAppFamilyBmp;
    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfTxSdmaGeneratorFlowIntervalTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Flow interval value change:
   While working in Flow interval mode change the cycle period and verify that
   transmission countinue acouring to the new configured rate.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorFlowIntervalChange)
{
    GT_U32 notAppFamilyBmp;
    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfTxSdmaGeneratorFlowIntervalChangeTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Rate adaptation for global throughput mode:
   Generator rate configured in global throughput mode is kept during packets
   addition and removal.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorGlobalThroughput)
{
    GT_U32 notAppFamilyBmp;
     /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

#ifdef ASIC_SIMULATION
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
#endif

    prvTgfNetIfTxSdmaGeneratorGlobalThroughputTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Burst transmission:
   Generator one shot burst transmission instead of continuous looping mode.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorBurstTx)
{
    GT_U32 notAppFamilyBmp;
     /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfTxSdmaGeneratorBurstTxTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Transition state:
   Verify that during the transition period due to linked list manipulations
   packets are not transmitted corruptedly.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorTransitionState)
{
    GT_U32 notAppFamilyBmp;
    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

#ifdef ASIC_SIMULATION
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
#endif

    prvTgfNetIfTxSdmaGeneratorTransitionStateTest();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Error scenarios:
   1. Prohibited linked list operations.
   2. Rate limits violation.
   3. Prohibited operations on non-generator queue.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaGeneratorErrorReport)
{
    GT_U32 notAppFamilyBmp;
    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC


    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfTxSdmaGeneratorErrorReportTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: RX SDMA performance test:
   1. For predefined list of packet sizes do the following:
   - Send wire-speed traffic to CPU
   - Check rate of packets in CPU.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfRxSdmaPerformance)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |UTF_CH2_E |UTF_CH3_E
                                                 |UTF_XCAT_E | UTF_XCAT3_E|UTF_XCAT2_E);

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfNetIfRxSdmaPerformanceTestInit();
    prvTgfNetIfRxSdmaPerformanceAllSizeTest(0,0,0, GT_FALSE, GT_FALSE);
    prvTgfNetIfRxSdmaPerformanceTestRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: TX SDMA performance test:
   1. For predefined list of packet sizes do the following:
   - Send huge burst of packets to port
   - Measure time that send take and calculate rate
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaPerformance)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |UTF_CH2_E |UTF_CH3_E
                                                 |UTF_XCAT_E | UTF_XCAT3_E|UTF_XCAT2_E);

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* use same init like for Rx SDMA test */
    prvTgfNetIfRxSdmaPerformanceTestInit();
    prvTgfNetIfTxSdmaPerformanceListTest();
    prvTgfNetIfRxSdmaPerformanceTestRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: RX to TX SDMA performance test:
   1. For predefined list of packet sizes do the following:
   - Send wire-speed traffic to CPU and TX from CPU to 10G port
   - Check rate of packets in CPU.
   - Check rate of packets in Tx port
*/
UTF_TEST_CASE_MAC(prvTgfNetIfRxToTxSdmaPerformance)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |UTF_CH2_E |UTF_CH3_E
                                                 |UTF_XCAT_E | UTF_XCAT3_E|UTF_XCAT2_E);

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfNetIfRxToTxSdmaPerformanceTestInit();
    prvTgfNetIfRxSdmaPerformanceAllSizeTest(0,0,0, GT_FALSE, GT_TRUE);
    prvTgfNetIfRxToTxSdmaPerformanceTestRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: RX SDMA All packet size test:
   1. For all packet sizes in range 64..10240 do the following:
   - Send wire-speed traffic to CPU
   - Check rate of packets in CPU.
   2. Test use minimal timeout for rate measurements.
   3. Check that RX rate of packets in CPU is not zero.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfRxSdmaAllSizes)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |UTF_CH2_E |UTF_CH3_E
                                                 |UTF_XCAT_E | UTF_XCAT3_E|UTF_XCAT2_E);

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfNetIfRxSdmaPerformanceTestInit();
    prvTgfNetIfRxSdmaPerformanceAllSizeTest(64,10240,1, GT_TRUE, GT_FALSE);
    prvTgfNetIfRxSdmaPerformanceTestRestore();
#endif
}

/* AUTODOC: NetIf event generation test
Checks event generation of:
* CPSS_PP_TX_BUFFER_QUEUE_E
* CPSS_PP_TX_END_QUEUE_E
The test performs following steps:
1. Bind interrupt handlers
2. Enable interrupts
3. Save defaults
4. Send testing traffic
5. Catch events
6. Restore defaults
*/
UTF_TEST_CASE_MAC(prvTgfNetIfTxSdmaEvents)
{
    GT_BOOL useMii;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_DXCH_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    useMii = ((PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode ==
               CPSS_NET_CPU_PORT_MODE_MII_E)  ? GT_TRUE :
              (PRV_CPSS_PP_MAC(prvTgfCpuDevNum)->cpuPortMode ==
               CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE :
    PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfCpuDevNum) ?
    GT_TRUE : GT_FALSE);

    if(useMii)
    {
        SKIP_TEST_MAC;
    }
    prvTgfNetIfTxSdmaEventsInit();
    prvTgfNetIfTxSdmaEventsGenerate();
    prvTgfNetIfTxSdmaEventsRestore();
}

UTF_TEST_CASE_MAC(prvTgfNetIfFromCpuToVidxWithExcludedPort)
{
    GT_U32      notAppFamilyBmp = 0;
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

{   /* skip multi device boards for cascading related tests */
    GT_U32      boardIdx, boardRevId, reloadEeprom;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }
}

    prvTgfNetIfFromCpuToVidxExcludedPort();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test - Queue implicit enabling and disabling
   Queue "remembers" user last enable or disable request.
   When enabling an empty queue the queue should not be "actually" move into
   enable state, but should be enabled (implicitly) only after the first packet
   addition to its list.
   Similarly, when the last packet from the list is removed and the queue was
   not explicitly disabled before it must be implicitly disabled.
*/
UTF_TEST_CASE_MAC(prvTgfNetIfSdmaRxQEnable)
{
    GT_U32 notAppFamilyBmp;

    /* feature not supported in GM therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfNetIfSdmaRxQEnableTest();
}

/*
 * Configuration of tgfPolicer suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfNetIf)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorLinkList)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorQEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorTwoQueues)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorFlowInterval)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorFlowIntervalChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorGlobalThroughput)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorBurstTx)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorTransitionState)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaGeneratorErrorReport)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfRxSdmaPerformance)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaPerformance)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfRxToTxSdmaPerformance)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfRxSdmaAllSizes)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfTxSdmaEvents)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfFromCpuToVidxWithExcludedPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfNetIfSdmaRxQEnable)
UTF_SUIT_END_TESTS_MAC(tgfNetIf)
