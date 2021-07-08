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
* @file prvTgfNetIfTxSdmaGeneratorGlobalThroughput.c
*
* @brief Tx SDMA Generator rate adaptation for flow interval testing
*
* @version   5
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

#include <netIf/prvTgfNetIfTxSdmaGeneratorGlobalThroughput.h>

/* port number to send traffic to by the Tx SDMA queue*/
#define PRV_TGF_SEND_PORT_IDX_CNS     2

/* Tx SDMA queue used for packet generator */
#define PRV_TGF_TX_SDMA_GENERATOR_Q_CNS     3

/* Tx SDMA queue packets per second rate */
#define PRV_TGF_TX_SDMA_GENERATOR_Q_PACKETS_PER_SECOND_CNS   12

/* Test traffic measuring time */
#define PRV_TGF_TX_SDMA_GENERATOR_TEST_INTERVAL_SECOND_CNS   1

/* total packet transmitted per interval */
#define PRV_TGF_TX_SDMA_GENERATOR_PACKETS_TX_CNS   (PRV_TGF_TX_SDMA_GENERATOR_Q_PACKETS_PER_SECOND_CNS * PRV_TGF_TX_SDMA_GENERATOR_TEST_INTERVAL_SECOND_CNS)

/* Packet Tx queue */
#define PRV_TGF_TX_QUEUE_CNS    5

/* Number of packets in generator queue chain  */
#define PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS  4

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

static GT_U8 packetsUpdateArr[PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS];

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfNetIfTxSdmaGeneratorGlobalThroughputTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorGlobalThroughputTestInit
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Reset ports counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, NULL, 0, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth");
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorGlobalThroughputTestRestore function
* @endinternal
*
* @brief   Restore test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorGlobalThroughputTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U64    rateValue;

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable Tx SDMA generator queue 1 */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q_CNS);

    /* AUTODOC: Set queue rate to NO GAP */
    rateValue.l[0] = rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                           rateValue,
                                           NULL);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                 0);
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketParamSet function
* @endinternal
*
* @brief   Set the various packet parameters (including DSA tag info) required for
*         packet addition and updating.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketParamSet
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
    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_Q_CNS;

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
* @internal prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsVerify function
* @endinternal
*
* @brief   Checking that the Tx packets are the correct ones, with the appropriate
*         updates if configured.
* @param[in] packetsCountArr          - (pointer to) array containing the expected packets
*                                      number indexed by packet ID.
* @param[in] packetsUpdateArr         - (pointer to) array cotaining packet data to
*                                      identify a packet.
* @param[in] callIdentifier           - identifier ID to distinguish calls to this check
*                                      function.
*                                       None
*
* @note Capturing to CPU is done in this function.
*
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsVerify
(
    GT_U32 *packetsCountArr,
    GT_U8  *packetsUpdateArr,
    GT_U32 callIdentifier
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetsCount[PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS];
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8     packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32    buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32    packetActualLength = 0;
    GT_U8     devNum;
    GT_U8     queue;
    TGF_NET_DSA_STC rxParam;

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    cpssOsMemSet(&packetsCount[0], 0, sizeof(packetsCount));

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* AUTODOC: Enable capture Tx */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 callIdentifier);

    /* AUTODOC: Wait test interval */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_TEST_INTERVAL_SECOND_CNS*1000);

    /* AUTODOC: Disable capture Tx */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 callIdentifier);

    /* Get first entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       GT_TRUE, GT_FALSE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);

    while( GT_NO_MORE != rc )
    {
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorRxInCpuGet: %d, %d, %d",
                                     TGF_PACKET_TYPE_CAPTURE_E,
                                     GT_FALSE,
                                     callIdentifier);

        for( ii = 0; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ; ii++ )
        {
            if( packetBuff[1] == packetsUpdateArr[ii] )
            {
                packetsCount[ii]++;
            }
        }

        /* Get next entry from captured packet's table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                           GT_FALSE, GT_FALSE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue,
                                           &rxParam);
    }

    UTF_VERIFY_EQUAL3_STRING_MAC(GT_NO_MORE, rc,
                                 "tgfTrafficGeneratorRxInCpuGet: %d, %d, %d",
                                 TGF_PACKET_TYPE_CAPTURE_E,
                                 GT_FALSE,
                                 callIdentifier);

    for( ii = 0; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ; ii++ )
    {
        if( ((packetsCountArr[ii] == 0) && (packetsCount[ii] != 0)) ||
            (packetsCountArr[ii] > packetsCount[ii]) ||
            (packetsCountArr[ii] < (packetsCount[ii] - 1)) )
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(packetsCountArr[ii],packetsCount[ii],
              "packet id[%d]: packets counted [%d] different then expected [%d], %d",
              ii, packetsCount[ii], packetsCountArr[ii], callIdentifier);
        }
    }
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsAdd function
* @endinternal
*
* @brief   Adding packets to the queues linked lists.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsAdd
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii, jj;
    GT_U32    packetId;
    GT_U64    rateValue;
    GT_U64    actualRateValue;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS];

    /* AUTODOC: Set queue rate to 12 packets per second */
    rateValue.l[0] = PRV_TGF_TX_SDMA_GENERATOR_Q_PACKETS_PER_SECOND_CNS;
    rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
                                           rateValue,
                                           &actualRateValue);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
                                 PRV_TGF_TX_SDMA_GENERATOR_Q_PACKETS_PER_SECOND_CNS);

    /* AUTODOC: Enable Tx SDMA generator for queue */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                 GT_FALSE, 0);

    prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketParamSet();

    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = 0;
    }

    /* AUTODOC: Add 4 distiguishable Packets to Queue */
    for ( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ; ii++)
    {
        /* AUTODOC: Add packet*/
        packetData[1] = packetsUpdateArr[ii] = (GT_U8)(ii+1);
        rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                                 &packetParams,
                                                 &packetData[0],
                                                 PRV_TGF_PACKET_LENGTH_CNS,
                                                 &packetId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                     prvTgfDevNum, ii);

        UTF_VERIFY_EQUAL2_STRING_MAC(ii, packetId,
                                     "different packet id [%d] then expected [%d]",
                                     packetId, ii);

#ifdef ASIC_SIMULATION
        /* Due to simulation transition from idle/disabled to running/enabled*/
        if( 0 == ii )
            cpssOsTimerWkAfter(1000);
#endif

        for( jj = 0 ; jj <= ii ; jj++ )
        {
            packetsCountArr[jj] =
                PRV_TGF_TX_SDMA_GENERATOR_PACKETS_TX_CNS/(ii+1);
        }

        /* AUTODOC: Verify expected periodic packets transmissions */
        prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsVerify(packetsCountArr,
                                                                packetsUpdateArr,
                                                                ii);
    }
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsRemove function
* @endinternal
*
* @brief   Removing packets from the linked lists.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsRemove
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii, jj;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS];

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ; ii > 0 ; ii--)
    {
        /* AUTODOC: Remove packet*/
        rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                                ii-1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, ii-1);

        /* After removal wait cycle for steady state */
        cpssOsTimerWkAfter((1000*PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS)/
                            PRV_TGF_TX_SDMA_GENERATOR_Q_PACKETS_PER_SECOND_CNS);

        for( jj = 0 ; jj < PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ; jj++ )
        {
            if( jj < ii-1 )
            {
                packetsCountArr[jj] =
                    PRV_TGF_TX_SDMA_GENERATOR_PACKETS_TX_CNS/(ii-1);
            }
            else
            {
                packetsCountArr[jj] = 0;
            }
        }

        /* AUTODOC: Verify expected periodic packets transmissions */
        prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsVerify(packetsCountArr,
                                                                packetsUpdateArr,
                                                                10+ii);
    }
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorGlobalThroughputTest function
* @endinternal
*
* @brief   Tx SDMA generator rate adaptation for global throughput test.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorGlobalThroughputTest
(
    GT_VOID
)
{
    prvTgfNetIfTxSdmaGeneratorGlobalThroughputTestInit();

    prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsAdd();

    prvTgfNetIfTxSdmaGeneratorGlobalThroughputPacketsRemove();

    prvTgfNetIfTxSdmaGeneratorGlobalThroughputTestRestore();
}

