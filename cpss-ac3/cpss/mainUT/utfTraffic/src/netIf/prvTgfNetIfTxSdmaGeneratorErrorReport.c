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
* @file prvTgfNetIfTxSdmaGeneratorErrorReport.c
*
* @brief Tx SDMA Generator error report testing
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

#include <netIf/prvTgfNetIfTxSdmaGeneratorErrorReport.h>

/* ports number to send traffic to by the Tx SDMA queues*/
#define PRV_TGF_SEND_Q1_PORT_IDX_CNS     1
#define PRV_TGF_SEND_Q2_PORT_IDX_CNS     2

/* Tx SDMA queues used for packet generator */
#define PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS     3
#define PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS     6

#define PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS     2

/* Packet Tx queue */
#define PRV_TGF_TX_QUEUE_CNS    7

static GT_U32 prv_tgf_num_of_packets_in_q1_chain = (512/2);
static GT_U32 prv_tgf_num_of_packets_in_q2_chain = (512/2);
/* Number of packets in generator queues chain  */
#define PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS prv_tgf_num_of_packets_in_q1_chain
#define PRV_TGF_NUM_OF_PACKETS_IN_Q2_CHAIN_CNS prv_tgf_num_of_packets_in_q2_chain

#define PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS (PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS+PRV_TGF_NUM_OF_PACKETS_IN_Q2_CHAIN_CNS)

/* Tx SDMA queue cycle length (in nanoseconds) - now 150ms*/
#define PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS     150000000

/* Big cycle configuration, based on at least 2 packets in chain 80s = 80,000,000,000ns = 0x12 A05F 2000*/
#define PRV_TGF_TX_SDMA_GENERATOR_QUEUE_BIG_CYCLE_NS_HIGH_WORD_CNS 0x12
#define PRV_TGF_TX_SDMA_GENERATOR_QUEUE_BIG_CYCLE_NS_LOW_WORD_CNS  0xA05F2000

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

/* Generator buffer size */
#define PRV_TGF_GEN_BUFFER_SIZE_CNS  (128 + TGF_eDSA_TAG_SIZE_CNS)

static PRV_TGF_NET_TX_PARAMS_STC packetParams;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfNetIfTxSdmaGeneratorErrorReportTestRestore function
* @endinternal
*
* @brief   Restore test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorErrorReportTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U64    rateValue;

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
* @internal prvTgfNetIfTxSdmaGeneratorErrorReportPacketParamSet function
* @endinternal
*
* @brief   Set the various packet parameters (including DSA tag info) required for
*         packet addition and updating.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorErrorReportPacketParamSet
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
* @internal prvTgfNetIfTxSdmaGeneratorErrorReportLinkedList function
* @endinternal
*
* @brief   Error report verifivation regarding linked list operations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorErrorReportLinkedList
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetId;

    prvTgfNetIfTxSdmaGeneratorErrorReportPacketParamSet();

    /* AUTODOC: Fill Q1 to its limit */
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
        packetData[1] = (GT_U8)ii;
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


    /* AUTODOC: Fill Q2 to its limit */
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
        packetData[1] = (GT_U8)ii;
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

    /* AUTODOC: Try adding another packet to full Q2 */
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_RESOURCE, rc,
                   "prvTgfNetIfSdmaTxGeneratorPacketAdd: Full Queue expected");

    /* AUTODOC: Try to update a packet belonging to Q1 in Q2 */
    packetId = 0;
    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorPacketUpdate: try to update packet [%d] in Q2 belonging to Q1",
        packetId);

    /* AUTODOC: Try to remove a packet belonging to Q1 from Q2 */
    packetId = 0;
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorPacketRemove: try to remove packet [%d] from Q2 belonging to Q1",
        packetId);

    /* AUTODOC: Try to update a packet that does not exists */
    packetId = PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS;
    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
          "prvTgfNetIfSdmaTxGeneratorPacketUpdate: try to update not existing packet [%d]",
                                 packetId);

    /* AUTODOC: Try to remove a packet that does not exists */
    packetId = PRV_TGF_NUM_OF_PACKETS_IN_QS_CHAIN_CNS;;
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorPacketRemove: try to remove not existing packet [%d]",
        packetId);

    /* AUTODOC: Empty Q2 packet chain */
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

    /* AUTODOC: Try to remove another packet from empty Q2 */
    packetId = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS;;
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                                packetId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorPacketRemove: try to remove not existing packet [%d] from empty Q2",
        packetId);

    /* AUTODOC: Try to add packet with length exceeding buffer size */
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
          PRV_TGF_GEN_BUFFER_SIZE_CNS + 1 - TGF_eDSA_TAG_SIZE_CNS,
                                             &packetId);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, rc,
                   "prvTgfNetIfSdmaTxGeneratorPacketAdd: packet length cannot exceed buffer size");
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorErrorReportRateLimits function
* @endinternal
*
* @brief   Error report due to rate limits violations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorErrorReportRateLimits
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    packetId;
    GT_U64    rateValue;
    GT_U64    actualRateValue;

    /* AUTODOC: Try to configure burst size above max size */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS,
                                          GT_TRUE, BIT_29);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorEnable: burst size over limit in queue [%d]",
        PRV_TGF_TX_SDMA_GENERATOR_Q1_CNS);

    prvTgfNetIfTxSdmaGeneratorErrorReportPacketParamSet();

    /* AUTODOC: Add 2 packets to Q2 */
    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum =
                                prvTgfPortsArray[PRV_TGF_SEND_Q2_PORT_IDX_CNS];
    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* we can not use the 'physical port' info --> use the ePort */
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams);
    }
    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS ;
          ii < PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS + 2 ;
          ii++)
    {
        packetData[1] = (GT_U8)ii;
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

    /* AUTODOC: Configure loop cycle max possible time based on 2 packets */
    rateValue.l[0] = PRV_TGF_TX_SDMA_GENERATOR_QUEUE_BIG_CYCLE_NS_LOW_WORD_CNS;
    rateValue.l[1] = PRV_TGF_TX_SDMA_GENERATOR_QUEUE_BIG_CYCLE_NS_HIGH_WORD_CNS;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                           rateValue,
                                           &actualRateValue);

    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "prvTgfNetIfSdmaTxGeneratorRateSet: Big rate set [0x%X%X]ns for queue [%d]",
        PRV_TGF_TX_SDMA_GENERATOR_QUEUE_BIG_CYCLE_NS_HIGH_WORD_CNS,
        PRV_TGF_TX_SDMA_GENERATOR_QUEUE_BIG_CYCLE_NS_LOW_WORD_CNS,
        PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS);

    /* AUTODOC: Try to remove 1 packet - will fail since rate configuration cannot be fulfiled */
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                                PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_VALUE, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS);

    /* AUTODOC: Change rate configured to enable packets removal */
    rateValue.l[0] = PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS;
    rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                           rateValue,
                                           &actualRateValue);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfNetIfSdmaTxGeneratorRateSet: New rate set for queue [%d]",
        PRV_TGF_TX_SDMA_GENERATOR_Q2_CNS);

    /* AUTODOC: Remove 2 packets for Q2 */
    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q1_CHAIN_CNS + 2 ;
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
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorErrorReportNonGenQueue function
* @endinternal
*
* @brief   Error report verifivation regarding Generator operations on queue not
*         configured as Generator.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorErrorReportNonGenQueue
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    packetId;
    GT_U64    rateValue;
    GT_U64    actualRateValue;
    PRV_TGF_NET_TX_GENERATOR_BURST_STATUS_ENT   burstStatus;

    prvTgfNetIfTxSdmaGeneratorErrorReportPacketParamSet();

    packetParams.sdmaInfo.txQueue = PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS;
    packetParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum =
                                prvTgfPortsArray[PRV_TGF_SEND_Q1_PORT_IDX_CNS];
    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        /* we can not use the 'physical port' info --> use the ePort */
        PRV_TGF_NET_TX_GENERATOR_FORCE_USE_EPORT_MAC(packetParams);
    }

    /* AUTODOC: Trial operations on non generator queue */

    /* AUTODOC: Prohibited packet addition */
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorPacketAdd: packet addition prohibited on non generator queue [%d]",
        PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);

    /* AUTODOC: Prohibited packet updating */
    rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                packetId,
                                                &packetParams,
                                                &packetData[0],
                                                PRV_TGF_PACKET_LENGTH_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorPacketUpdate: packet updating prohibited on non generator queue [%d]",
        PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);

    /* AUTODOC: Prohibited packet removal */
    rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS,
                                                0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorPacketRemove: packet removal prohibited on non generator queue [%d]",
        PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);

    /* AUTODOC: Prohibited rate configuration */
    rateValue.l[0] = PRV_TGF_TX_SDMA_GENERATOR_QUEUE_CYCLE_NS_CNS;
    rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                           rateValue,
                                           &actualRateValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorRateSet: rate set prohibited on non generator queue [%d]",
        PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);

    /* AUTODOC: Prohibited queue enabling */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
        "prvTgfNetIfSdmaTxGeneratorEnable: gen queue enabling prohibited on non generator queue [%d]",
        PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);

    /* AUTODOC: Prohibited queue disabling */
    rc = prvTgfNetIfSdmaTxGeneratorDisable(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
                                 "prvTgfNetIfSdmaTxGeneratorDisable: gen queue disabling prohibited on non generator queue [%d]",
                                 PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);

    /* AUTODOC: Prohibited burst status retrival */
    prvTgfNetIfSdmaTxGeneratorBurstStatusGet(prvTgfDevNum,
                                             PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS,
                                             &burstStatus);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, rc,
                                 "prvTgfNetIfSdmaTxGeneratorBurstStatusGet: burst status get prohibited on non generator queue [%d]",
                                 PRV_TGF_TX_SDMA_NON_GENERATOR_Q_CNS);
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorErrorReportTest function
* @endinternal
*
* @brief   Tx SDMA generator error reports test.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorErrorReportTest
(
    GT_VOID
)
{
    GT_U32 tmpData;
    if(appDemoDbEntryGet("txGenNumOfDescBuff", &tmpData) == GT_OK)
    {
        /* !!! support falcon 12.8/6.4 that uses less descriptors !!! */
        prv_tgf_num_of_packets_in_q1_chain = tmpData / 2;
    }
    else
    {
        /* 512 is default by the appDemoAllocateDmaMem(...) */
        prv_tgf_num_of_packets_in_q1_chain  = 512 / 2;
    }
    prv_tgf_num_of_packets_in_q2_chain = prv_tgf_num_of_packets_in_q1_chain;

    prvTgfNetIfTxSdmaGeneratorErrorReportLinkedList();

    prvTgfNetIfTxSdmaGeneratorErrorReportRateLimits();

    prvTgfNetIfTxSdmaGeneratorErrorReportNonGenQueue();

    prvTgfNetIfTxSdmaGeneratorErrorReportTestRestore();
}

