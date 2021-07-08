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
* @file prvTgfNetIfTxSdmaGeneratorLinkList.c
*
* @brief Tx SDMA Generator linked list various operations testing
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

#include <netIf/prvTgfNetIfTxSdmaGeneratorLinkList.h>

/* port number to send traffic to by the Tx SDMA */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* Tx SDMA queue used for packet generator */
#define PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS     3

/* Tx SDMA queue cycle length (in nanoseconds) - now 150ms*/
#define PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS     150000000

/* Packet Tx queue */
#define PRV_TGF_TX_QUEUE_CNS    7

/* Number of packets in generator queue chain  */
#define PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS 5

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

static GT_U8 packetsUpdateArr[PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS];

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfNetIfTxSdmaGeneratorLinkListTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListTestInit
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
* @internal prvTgfNetIfTxSdmaGeneratorLinkListTestRestore function
* @endinternal
*
* @brief   Restore test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListTestRestore
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

    /* AUTODOC: Disable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

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

    /* AUTODOC: Arranging free packet ID list to be in order */
    for ( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii++)
    {
        rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                                 &packetParams,
                                                 &packetData[0],
                                                 PRV_TGF_PACKET_LENGTH_CNS,
                                                 &packetId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                     prvTgfDevNum, ii);
    }

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii > 0 ; ii--)
    {
        rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                ii-1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, ii-1);
    }
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorLinkListPacketParamSet function
* @endinternal
*
* @brief   Set the various packet parameters (including DSA tag info) required for
*         packet addition and updating.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListPacketParamSet
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
* @internal prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify function
* @endinternal
*
* @brief   Checking that the Tx packets are the correct ones, with the appropriate
*         updates if configured.
* @param[in] packetsCountArr          - (pointer to) array containing the expected packets
*                                      number indexed by packet ID.
* @param[in] packetsUpdateArr         - (pointer to) array cotaining packet data to
*                                      identify a packet.
* @param[in] chainCyclesToWait        - the approximate chain cycles to wait.
* @param[in] callIdentifier           - identifier ID to distinguish calls to this check
*                                      function.
*                                       None
*
* @note Capturing to CPU is done in this function.
*
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify
(
    GT_U32 *packetsCountArr,
    GT_U8  *packetsUpdateArr,
    GT_U32 chainCyclesToWait,
    GT_U32 callIdentifier
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetsCount[PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS];
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

    /* AUTODOC: Chain cycles wait */
    cpssOsTimerWkAfter((PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000)*
                                                            (chainCyclesToWait));

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

        for( ii = 0; ii < PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii++ )
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

    for( ii = 0; ii < PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii++ )
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
* @internal prvTgfNetIfTxSdmaGeneratorLinkListPacketsAdd function
* @endinternal
*
* @brief   Adding packets to the linked list and enable queue.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListPacketsAdd
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetId;
    GT_U64    rateValue;
    GT_U64    actualRateValue;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS];
    GT_U32    chainCyclesNum = 3;
    PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_ENT checkResult;

    prvTgfNetIfTxSdmaGeneratorLinkListPacketParamSet();

    /* AUTODOC: Add 5 distiguishable Packets, their packets ID will be 0-4 */
    for ( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii++)
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

    /* AUTODOC: Set queue rate to finish cycle every 150 ms */
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

    /* AUTODOC: Enable Tx SDMA generator queue */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                 GT_FALSE, 0);

    /* AUTODOC: Verify expected periodic transmission */
    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = chainCyclesNum;
    }

#ifdef ASIC_SIMULATION
    /* Due to simulation idle if queue is disabled */
    cpssOsTimerWkAfter(1000);
#endif

    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    0);

    /* AUTODOC: Verify memory integrity for queue */
    rc = prvTgfNetIfSdmaTxGeneratorMemCheck(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                          &checkResult);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorMemCheck for queue %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E,
                                 checkResult,
                                 "generator queue [%d] generator memory check failed",
                                 PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS);
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorLinkListPacketsUpdate function
* @endinternal
*
* @brief   Updating packets in the linked list.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListPacketsUpdate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS];
    GT_U32    chainCyclesNum = 2;
    GT_U32    packetId;

    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = chainCyclesNum;
    }

    /* AUTODOC: Upadte packet ID 3 */
    packetId = 3;
    packetData[1] = packetsUpdateArr[packetId] = 13;

    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    1);

    /* AUTODOC: Upadte packet ID 4 (now last in chain) */
    packetId = 4;
    packetData[1] = packetsUpdateArr[packetId] = 14;

    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    2);

    /* AUTODOC: Upadte packet ID 0 (now first in chain) */
    packetId = 0;
    packetData[1] = packetsUpdateArr[packetId] = 10;

    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    3);

    /* AUTODOC: Upadte packet ID 2 */
    packetId = 2;
    packetData[1] = packetsUpdateArr[packetId] = 12;

    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    4);

    /* AUTODOC: Upadte packet ID 1 */
    packetId = 1;
    packetData[1] = packetsUpdateArr[packetId] = 11;

    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    5);
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorLinkListPacketsRemove function
* @endinternal
*
* @brief   Removing packets from the linked list.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListPacketsRemove
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetsCountArr[PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS];
    GT_U32    chainCyclesNum = 2;
    GT_U32    packetId;

    for( ii = 0 ; ii < PRV_TGF_NUM_OF_PACKETS_IN_CHAIN_CNS ; ii++ )
    {
        packetsCountArr[ii] = chainCyclesNum;
    }

    /* AUTODOC: Remove packet ID 2 */
    packetId = 2;
    packetsCountArr[packetId] = 0;

    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);

    /* After removal wait cycle for steady state */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    11);

    /* AUTODOC: Remove packet ID 3 (now last in chain) */
    packetId = 3;
    packetsCountArr[packetId] = 0;

    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);

    /* After removal wait cycle for steady state */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    12);

    /* AUTODOC: Remove packet ID 0 (now first in chain) */
    packetId = 0;
    packetsCountArr[packetId] = 0;

    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);

    /* After removal wait cycle for steady state */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    13);

    /* AUTODOC: Remove packet ID 1 (now last in chain) */
    packetId = 1;
    packetsCountArr[packetId] = 0;

    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);

    /* After removal wait cycle for steady state */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    14);

    /* AUTODOC: Remove packet ID 4 (now last and first in chain) */
    packetId = 4;
    packetsCountArr[packetId] = 0;

    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);

    /* After removal wait cycle for steady state */
    cpssOsTimerWkAfter(PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS/1000000);

    /* AUTODOC: Verify expected periodic transmission */
    prvTgfNetIfTxSdmaGeneratorLinkListPacketsVerify(packetsCountArr,
                                                    packetsUpdateArr,
                                                    chainCyclesNum,
                                                    12);
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorLinkListTest function
* @endinternal
*
* @brief   Tx SDMA generator linked list manipulations (various operations) test.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorLinkListTest
(
    GT_VOID
)
{
    prvTgfNetIfTxSdmaGeneratorLinkListTestInit();

    prvTgfNetIfTxSdmaGeneratorLinkListPacketsAdd();

    prvTgfNetIfTxSdmaGeneratorLinkListPacketsUpdate();

    prvTgfNetIfTxSdmaGeneratorLinkListPacketsRemove();

    prvTgfNetIfTxSdmaGeneratorLinkListTestRestore();
}

