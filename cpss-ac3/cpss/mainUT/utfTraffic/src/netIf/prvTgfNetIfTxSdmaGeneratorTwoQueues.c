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
* @file prvTgfNetIfTxSdmaGeneratorTwoQueues.c
*
* @brief Tx SDMA Generator two queues testing
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

#include <netIf/prvTgfNetIfTxSdmaGeneratorTwoQueues.h>

/* ports number to send traffic to by the Tx SDMA queues*/
#define PRV_TGF_SEND_Q1_PORT_IDX_CNS     1
#define PRV_TGF_SEND_Q2_PORT_IDX_CNS     2

/* Tx SDMA queues used for packet generator */
#define PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS     3
#define PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS     6

/* Tx SDMA queue cycle length (in nanoseconds) - Q1:125ms, Q2:100ms*/
/* => 1/2 second contains 4 cycles of Q1, 5 cycles of Q2 */
#define PRV_TGF_TX_SDMA_GENERATOR_Q1_CYCLE_NS_CNS     125000000
#define PRV_TGF_TX_SDMA_GENERATOR_Q2_CYCLE_NS_CNS     100000000

/* Packet Tx queue */
#define PRV_TGF_TX_QUEUE_CNS    7

/* Number of packets in generator queues chain  */
#define PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS 5
#define PRV_TGF_NUM_OF_PACKETS_IN_Q2_CHAIN_CNS 4

/* Q1 packets ID 0-4, Q2 packets ID 5-8 */
#define PRV_TGF_UPDATE_PACKET_ID_IN_Q1_CNS 1
#define PRV_TGF_UPDATE_PACKET_ID_IN_Q2_CNS 7
#define PRV_TGF_REMOVE_PACKET_ID_IN_Q1_CNS 2
#define PRV_TGF_REMOVE_PACKET_ID_IN_Q2_CNS 6

#define PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS (PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS+PRV_TGF_NUM_OF_PACKETS_IN_Q2_CHAIN_CNS)

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

static GT_U8 packetsUpdateArr[PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS];

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesTestInit
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
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesTestRestore function
* @endinternal
*
* @brief   Restore test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetId;
    GT_U64    rateValue;

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable Tx SDMA generator queue 1 */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);

    /* AUTODOC: Disable Tx SDMA generator queue 2 */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);

    /* AUTODOC: Set queue rate to NO GAP */
    rateValue.l[0] = rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                           rateValue,
                                           NULL);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                 0);

    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                           rateValue,
                                           NULL);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                 0);

    /* AUTODOC: Arranging free packet ID list to be in order */

    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS;

    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS);

    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_REMOVE_PACKET_ID_IN_Q2_CNS, packetId,
                                 "different packet id [%d] then expected [%d]",
                                 packetId, PRV_TGF_REMOVE_PACKET_ID_IN_Q2_CNS);

    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS;

    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);

    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_REMOVE_PACKET_ID_IN_Q1_CNS, packetId,
                                 "different packet id [%d] then expected [%d]",
                                 packetId, PRV_TGF_REMOVE_PACKET_ID_IN_Q1_CNS);


    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ;
          ii > PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii--)
    {
        rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                                ii-1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, ii-1);
    }

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii > 0 ;
          ii--)
    {
        rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                                ii-1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, ii-1);
    }
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketParamSet function
* @endinternal
*
* @brief   Set the various packet parameters (including DSA tag info) required for
*         packet addition and updating.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketParamSet
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
    packetParams.dsaParam.dsaInfo.fromCpu.tc = PRV_TGF_TX_QUEUE_CNS;
    packetParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* we can not use the 'physical port' info --> use the ePort */
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams);
    }
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify function
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
* @note Capturing to CPU is done in this function for a period of 1 second.
*
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify
(
    GT_U32 *packetsCountArr,
    GT_U8  *packetsUpdateArr,
    GT_U32 callIdentifier
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetsCount[PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS];
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8     packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32    buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32    packetActualLength = 0;
    GT_U8     devNum;
    GT_U8     queue;
    TGF_NET_DSA_STC rxParam;
    GT_U32    maxPacketsCount = 0;
    GT_U32    minPacketsCount = 0;
    GT_BOOL   minMaxInit = GT_TRUE;

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    cpssOsMemSet(&packetsCount[0], 0, sizeof(packetsCount));

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS];

    /* AUTODOC: Enable capture Tx for traffic from Q1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS],
                                 callIdentifier);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_Q2_PORT_IDX_CNS];

    /* AUTODOC: Enable capture Tx for traffic from Q2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_Q2_PORT_IDX_CNS],
                                 callIdentifier);

    /* AUTODOC: 1/2 second wait - 4 cycles of Q1, 5 cycles of Q2 */
    cpssOsTimerWkAfter(500);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS];

    /* AUTODOC: Disable capture Tx for traffic from Q1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE );
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS],
                                 callIdentifier);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_Q2_PORT_IDX_CNS];

    /* AUTODOC: Disable capture Tx for traffic from Q2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_Q2_PORT_IDX_CNS],
                                 callIdentifier);

    /* provide some time for simulation tasks related to packets to CPU */
#ifdef ASIC_SIMULATION
    cpssOsTimerWkAfter(500);
#endif

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

        for( ii = 0; ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ; ii++ )
        {
            if( packetBuff[1] == packetsUpdateArr[ii] )
            {
                packetsCount[ii]++;
                break;
            }
        }

        if( ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS )
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(
                prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS],
                rxParam.portNum,
                "packet capture indicate wrong port, %d\n",
                callIdentifier);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(
                prvTgfPortsArray[PRV_TGF_SEND_Q2_PORT_IDX_CNS],
                rxParam.portNum,
                "packet capture indicate wrong port, %d\n",
                callIdentifier);
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

    for( ii = 0; ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ; ii++ )
    {
        if( packetsCount[ii] !=0 )
        {
            if( GT_TRUE == minMaxInit )
            {
                maxPacketsCount = packetsCount[ii];
                minPacketsCount = packetsCount[ii];
            }
            else /* GT_FALSE == minMaxInit */
            {
                if( maxPacketsCount < packetsCount[ii] )
                {
                    maxPacketsCount = packetsCount[ii];
                }

                if( minPacketsCount > packetsCount[ii] )
                {
                    minPacketsCount = packetsCount[ii];
                }
            }
        }

        if( ((packetsCountArr[ii] == 0) && (packetsCount[ii] != 0)) ||
            (packetsCountArr[ii] > packetsCount[ii]) ||
            (packetsCountArr[ii] < (packetsCount[ii] - 2)) )
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(packetsCountArr[ii],packetsCount[ii],
              "packet id[%d]: packets counted [%d] different then expected [%d], %d",
              ii, packetsCount[ii], packetsCountArr[ii], callIdentifier);
        }
    }

    if ((maxPacketsCount - maxPacketsCount) > 1)
    {
        UTF_VERIFY_EQUAL3_STRING_MAC(maxPacketsCount,maxPacketsCount,
              "difference betwwen max [%d] to min [%d] of different packtes in loop more then 1, %d",
              maxPacketsCount,maxPacketsCount,callIdentifier);
    }
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsAdd function
* @endinternal
*
* @brief   Adding packets to the queues linked lists.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsAdd
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetId;
    GT_U64    rateValue;
    GT_U64    actualRateValue;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS];
    PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_ENT checkResult;

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketParamSet();

    /* AUTODOC: Add 5 distiguishable Packets for Q1, their packets ID will be 0-4 */
    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum =
                                prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS];
    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* we can not use the 'physical port' info --> use the ePort */
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams);
    }
    for ( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ; ii++)
    {
        packetData[1] = packetsUpdateArr[ii] = (GT_U8)ii;
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
    }

    /* AUTODOC: Add 4 distiguishable Packets for Q2, their packets ID will be 5-8 */
    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum =
                                prvTgfPortsArray[PRV_TGF_SEND_Q2_PORT_IDX_CNS];
    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* we can not use the 'physical port' info --> use the ePort */
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams);
    }
    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ;
          ii++)
    {
        packetData[1] = packetsUpdateArr[ii] = (GT_U8)ii;
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
    }

    /* AUTODOC: Set queue 1 cycle to 125 ms */
    rateValue.l[0] = PRV_TGF_TX_SDMA_GENERATOR_Q1_CYCLE_NS_CNS;
    rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                           rateValue,
                                           &actualRateValue);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CYCLE_NS_CNS);

    /* AUTODOC: Set queue 2 cycle to 100 ms */
    rateValue.l[0] = PRV_TGF_TX_SDMA_GENERATOR_Q2_CYCLE_NS_CNS;
    rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                           rateValue,
                                           &actualRateValue);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                 PRV_TGF_TX_SDMA_GENERATOR_Q2_CYCLE_NS_CNS);

    /* AUTODOC: Enable Tx SDMA generator for queue 1 only */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                 GT_FALSE, 0);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Verify expected periodic transmission */
    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = 4;
    }

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ;
          ii++)
    {
        packetsCountArr[ii] = 0;
    }

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify(packetsCountArr,
                                                     packetsUpdateArr,
                                                     0);

    /* AUTODOC: Disable Tx SDMA generator queue 1 */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);

    /* AUTODOC: Enable Tx SDMA generator for queue 2 only */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                 GT_FALSE, 0);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Verify expected periodic transmission */
    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = 0;
    }

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ;
          ii++)
    {
        packetsCountArr[ii] = 5;
    }

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify(packetsCountArr,
                                                     packetsUpdateArr,
                                                     1);

    /* AUTODOC: Enable Tx SDMA generator for queue 1 */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                 GT_FALSE, 0);

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    /* AUTODOC: Verify expected periodic transmission */
    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = 4;
    }

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ;
          ii++)
    {
        packetsCountArr[ii] = 5;
    }

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify(packetsCountArr,
                                                     packetsUpdateArr,
                                                     2);

    /* AUTODOC: Verify memory integrity for queue 1 */
    rc = prvTgfNetIfSdmaTxGeneratorMemCheck(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                          &checkResult);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorMemCheck for queue %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E,
                                 checkResult,
                                 "generator queue [%d] generator memory check failed",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);

    /* AUTODOC: Verify memory integrity for queue 2 */
    rc = prvTgfNetIfSdmaTxGeneratorMemCheck(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                          &checkResult);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorMemCheck for queue %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E,
                                 checkResult,
                                 "generator queue [%d] generator memory check failed",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS);
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsUpdate function
* @endinternal
*
* @brief   Updating packets in queues linked lists.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsUpdate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS];
    GT_U32    packetId;


    /* AUTODOC: Upadte packet ID 7 in Q2*/
    packetId = PRV_TGF_UPDATE_PACKET_ID_IN_Q2_CNS;
    packetData[1] = packetsUpdateArr[packetId] = (GT_U8)(10 + packetId);

    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Verify expected periodic transmission */
    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = 4;
    }

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ;
          ii++)
    {
        packetsCountArr[ii] = 5;
    }

    /* make sure that old packet already updated  */
    cpssOsTimerWkAfter(125);

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify(packetsCountArr,
                                                     packetsUpdateArr,
                                                     10);

    /* AUTODOC: Upadte packet ID 1 in Q1*/
    packetId = PRV_TGF_UPDATE_PACKET_ID_IN_Q1_CNS;
    packetData[1] = packetsUpdateArr[packetId] = (GT_U8)(10 + packetId);

    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum =
                                prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS];
    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* we can not use the 'physical port' info --> use the ePort */
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams);
    }

    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);

    /* make sure that old packet already updated  */
    cpssOsTimerWkAfter(125);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify(packetsCountArr,
                                                     packetsUpdateArr,
                                                     11);
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsRemove function
* @endinternal
*
* @brief   Removing packets from the linked lists.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsRemove
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS];

    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = 4;
    }

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii < PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS ;
          ii++)
    {
        packetsCountArr[ii] = 5;
    }

    /* AUTODOC: Remove packet ID 2 from Q1 */
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                                PRV_TGF_REMOVE_PACKET_ID_IN_Q1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, PRV_TGF_REMOVE_PACKET_ID_IN_Q1_CNS);

    /* After removal wait cycle for steady state */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_Q1_CYCLE_NS_CNS/1000000);

    /* AUTODOC: Verify expected periodic transmission */
    packetsCountArr[PRV_TGF_REMOVE_PACKET_ID_IN_Q1_CNS] = 0;
    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify(packetsCountArr,
                                                     packetsUpdateArr,
                                                     20);

    /* AUTODOC: Remove packet ID 6 from Q1 */
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                                PRV_TGF_REMOVE_PACKET_ID_IN_Q2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, PRV_TGF_REMOVE_PACKET_ID_IN_Q2_CNS);

    /* After removal wait cycle for steady state */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_Q2_CYCLE_NS_CNS/1000000);

    /* AUTODOC: Verify expected periodic transmission */
    packetsCountArr[PRV_TGF_REMOVE_PACKET_ID_IN_Q2_CNS] = 0;
    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsVerify(packetsCountArr,
                                                     packetsUpdateArr,
                                                     21);
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorTwoQueuesTest function
* @endinternal
*
* @brief   Tx SDMA generator two queues manipulations (various operations) test.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorTwoQueuesTest
(
    GT_VOID
)
{
    prvTgfNetIfTxSdmaGeneratorTwoQueuesTestInit();

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsAdd();

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsUpdate();

    prvTgfNetIfTxSdmaGeneratorTwoQueuesPacketsRemove();

    prvTgfNetIfTxSdmaGeneratorTwoQueuesTestRestore();
}

