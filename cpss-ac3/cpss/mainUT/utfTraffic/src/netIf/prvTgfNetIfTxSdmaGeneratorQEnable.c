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
* @file prvTgfNetIfTxSdmaGeneratorQEnable.c
*
* @brief Tx SDMA Generator Queue enabling and disabling implicit testing
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>


#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfNetIfGen.h>

#include <netIf/prvTgfNetIfTxSdmaGeneratorQEnable.h>

/* port number to send traffic to by the Tx SDMA */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* Tx SDMA queue used for packet generator */
#define PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS     3

/* Tx SDMA queue cycle length (in nanoseconds) - now 80ms*/
#define PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS     80000000

/* Packet Tx queue */
#define PRV_TGF_TX_QUEUE_CNS    7

static GT_U8 packetData[]  =
                       {0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
                        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                        0x33, 0x33,
                        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};

/* Packet length */
#define PRV_TGF_PACKET_LENGTH_CNS    64

static PRV_TGF_NET_TX_PARAMS_STC packetParams;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfNetIfTxSdmaGeneratorQEnableTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorQEnableTestInit
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorQEnableTestRestore function
* @endinternal
*
* @brief   Restore test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorQEnableTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U64    rateValue;

    /* AUTODOC: Disable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    /* AUTODOC: Remove packet from the queue */
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, 0);

    /* AUTODOC: Set queue rate to NO GAP */
    rateValue.l[0] = rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                           rateValue,
                                           NULL);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                 0);
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorQEnablePacketParamSet function
* @endinternal
*
* @brief   Set the various packet parameters (including DSA tag info) required for
*         packet addition.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorQEnablePacketParamSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_HW_DEV_NUM hwDevNum;

    /* AUTODOC: Clear packet parameters */
    cpssOsMemSet(&packetParams, 0, sizeof(PRV_TGF_NET_TX_PARAMS_STC));
    packetParams.packetIsTagged = GT_FALSE;

    /* AUTODOC: Packet Tx SDMA info */
    packetParams.sdmaInfo.recalcCrc = GT_TRUE;
    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS;

    /* AUTODOC: Packet DSA tag info */
    packetParams.dsaParam.dsaType = TGF_DSA_4_WORD_TYPE_E;
    packetParams.dsaParam.dsaCommand = TGF_DSA_CMD_FROM_CPU_E;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    /* Get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum =
                                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    packetParams.dsaParam.dsaInfo.fromCpu.tc = PRV_TGF_TX_QUEUE_CNS;
    packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* we can not use the 'physical port' info --> use the ePort */
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams);
    }
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            - sending port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters
(
    IN GT_U32   sendPortIndex,
    IN GT_U32   expectedValue,
    IN GT_U32   callIdentifier
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if(sendPortIndex == portIter)
        {
            if( ((expectedValue == 0) && (portCntrs.goodPktsSent.l[0] != 0)) ||
                (expectedValue > portCntrs.goodPktsSent.l[0]) ||
                (expectedValue < (portCntrs.goodPktsSent.l[0] - 1)) )
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(expectedValue,
                                             portCntrs.goodPktsSent.l[0],
                   "port [%d]: Tx packets counted [%d] different then expected [%d], %d",
                                             prvTgfPortsArray[sendPortIndex],
                                             portCntrs.goodPktsSent.l[0],
                                             expectedValue,
                                             callIdentifier);
            }
        }
        else
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL4_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "Tx another goodPktsSent counter [%d], than expected [%d], on port [%d], %d",
                                         portCntrs.goodPktsSent.l[0], 0,
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
    }
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorQEnableDisable function
* @endinternal
*
* @brief   Test flow of enabling and disabling.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorQEnableDisable
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    packetId;
    GT_U64    rateValue;
    GT_U64    actualRateValue;
    GT_U32    chainCyclesNum;
    GT_BOOL   enable;

    prvTgfNetIfTxSdmaGeneratorQEnablePacketParamSet();

    /* AUTODOC: Enable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                 GT_FALSE, 0);

    /* AUTODOC: Verify HW queue is actually disabled */
    rc =  prvTgfNetIfSdmaTxQueueEnableGet(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                          &enable);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxQueueEnableGet: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(enable, GT_FALSE,
                                 "Expected queue [%d] implicitly disabled",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    /* AUTODOC: Set queue rate to finish cycle every 200 ms */
    rateValue.l[0] = PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS;
    rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                           rateValue,
                                           &actualRateValue);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 3;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));
    /* AUTODOC: Verify no transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       0, 0);

    /* AUTODOC: Add packet to the queue */
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d",
                                     prvTgfDevNum);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 3;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));

    /* AUTODOC: Verify periodic transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       chainCyclesNum, 1);

    /* AUTODOC: Remove packet from the queue */
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Verify HW queue is actually disabled */
    rc =  prvTgfNetIfSdmaTxQueueEnableGet(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                          &enable);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxQueueEnableGet: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(enable, GT_FALSE,
                                 "Expected queue [%d] implicitly disabled",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 3;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));
    /* AUTODOC: Verify no transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       0, 2);

    /* AUTODOC: Add packet to the queue */
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d",
                                     prvTgfDevNum);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 3;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));

    /* AUTODOC: Verify periodic transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       chainCyclesNum, 3);

    /* AUTODOC: Disable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 4;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));
    /* AUTODOC: Verify no transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       0, 4);

    /* AUTODOC: Enable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                 GT_FALSE, 0);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 2;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));

    /* AUTODOC: Verify periodic transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       chainCyclesNum, 5);

    /* AUTODOC: Disable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 3;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));
    /* AUTODOC: Verify no transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       0, 6);

    /* AUTODOC: Remove packet from the queue */
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 3;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));
    /* AUTODOC: Verify no transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       0, 7);

    /* AUTODOC: Add packet to the queue */
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d",
                                     prvTgfDevNum);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 5;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));
    /* AUTODOC: Verify no transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       0, 8);

    /* AUTODOC: Enable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                 GT_FALSE, 0);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: Chain cycles wait */
    chainCyclesNum = 5;
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesNum));

    /* AUTODOC: Verify periodic transmission */
    prvTgfNetIfTxSdmaGeneratorQEnableTestCheckCounters(PRV_TGF_SEND_PORT_IDX_CNS,
                                                       chainCyclesNum, 9);
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorQEnableTest function
* @endinternal
*
* @brief   Tx SDMA generator queue enable and disable implicitly test.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorQEnableTest
(
    GT_VOID
)
{
    prvTgfNetIfTxSdmaGeneratorQEnableTestInit();

    prvTgfNetIfTxSdmaGeneratorQEnableDisable();

    prvTgfNetIfTxSdmaGeneratorQEnableTestRestore();
}


