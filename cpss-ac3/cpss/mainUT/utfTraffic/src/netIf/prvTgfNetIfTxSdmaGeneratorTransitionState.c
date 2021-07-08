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
* @file prvTgfNetIfTxSdmaGeneratorTransitionState.c
*
* @brief Tx SDMA Generator transition state testing
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
#include <common/tgfPortGen.h>

#include <netIf/prvTgfNetIfTxSdmaGeneratorTransitionState.h>

/* port number to send traffic to by the Tx SDMA queue*/
#define PRV_TGF_SEND_PORT_IDX_CNS     2

/* Tx SDMA queue used for packet generator */
#define PRV_TGF_TX_SDMA_GENERATOR_Q_CNS     3

/* Packet Tx queue */
#define PRV_TGF_TX_QUEUE_CNS    5

/* Number of packets in generator queue chain  */
#define PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS 3

static GT_U8 packetData0[]  =
                       {0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
                        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                        0x03, 0x03,
                        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};

static GT_U8 packetData1[]  =
                       {0x00, 0x01, 0x12, 0x13, 0x14, 0x15,
                        0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                        0x13, 0x13,
                        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                        0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
                        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
                        0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
                        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
                        0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
                        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
                        0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F};

static GT_U8 packetData2[]  =
                       {0x00, 0x02, 0x22, 0x23, 0x24, 0x25,
                        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                        0x23, 0x23,
                        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
                        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
                        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
                        0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
                        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
                        0xa8, 0xa9, 0xaA, 0xaB, 0xaC, 0xaD, 0xaE, 0xaF,
                        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
                        0xb8, 0xb9, 0xbA, 0xbB, 0xbC, 0xbD, 0xbE, 0xbF};

/* Packet length */
#define PRV_TGF_PACKET_LENGTH_CNS    64

static PRV_TGF_NET_TX_PARAMS_STC packetParams;

static GT_U8 packetsUpdateArr[PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS];

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfNetIfTxSdmaGeneratorTransitionStateTestInit function
* @endinternal
*
* @brief   Various test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTransitionStateTestInit
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
* @internal prvTgfNetIfTxSdmaGeneratorTransitionStateTestRestore function
* @endinternal
*
* @brief   Restore test initializations
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTransitionStateTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    ii;
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

    for ( ii = PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ; ii > 0 ; ii--)
    {
        /* AUTODOC: Remove packet*/
        rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                                ii-1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, ii-1);
    }
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorTransitionStatePacketParamSet function
* @endinternal
*
* @brief   Set the various packet parameters (including DSA tag info) required for
*         packet addition and updating.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTransitionStatePacketParamSet
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
* @internal prvTgfNetIfTxSdmaGeneratorTransitionStateVerify function
* @endinternal
*
* @brief   Checking that the Tx packets are the correct ones, with the appropriate
*         updates if configured.
* @param[in] packetId                 - packet ID to add or remove.
* @param[in] removeAddOrUpdate        - 0 for remove packet,
*                                      1 for add packet,
*                                      2 for update packet
* @param[in] callIdentifier           - identifier ID to distinguish calls to this check
*                                      function.
*                                       None
*
* @note Remove or add packet, capturing to CPU is done in this function.
*
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTransitionStateVerify
(
    GT_U32 packetId,
    GT_U32 removeAddOrUpdate,
    GT_U32 callIdentifier
)
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8     packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32    buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32    packetActualLength = 0;
    GT_U8     devNum;
    GT_U8     queue;
    TGF_NET_DSA_STC rxParam;
    GT_U8* packetBuffPtr = NULL;
    GT_U32 getPacketId;

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* Enable capture Tx */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 callIdentifier);

    switch(packetId)
    {
        case 0: packetBuffPtr = &packetData0[0];
                break;
        case 1: packetBuffPtr = &packetData1[0];
                break;
        case 2: packetBuffPtr = &packetData2[0];
                break;
        default:
                UTF_VERIFY_EQUAL1_STRING_MAC(0, packetId,
                                 "Unexpected packet ID: %d", packetId);
                break;
    }

    switch( removeAddOrUpdate )
    {
        case 0:
        /*Remove packet*/
        rc = prvTgfNetIfSdmaTxGeneratorPacketRemove(prvTgfDevNum,
                                                PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                                packetId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketRemove: %d, %d",
                                 prvTgfDevNum, packetId);
            break;
        case 1:
        /* Add packet */
        rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                                 &packetParams,
                                                 packetBuffPtr,
                                                 PRV_TGF_PACKET_LENGTH_CNS,
                                                 &getPacketId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                     prvTgfDevNum, packetId);

        UTF_VERIFY_EQUAL2_STRING_MAC(packetId, getPacketId,
                                     "different packet id [%d] then expected [%d]",
                                     getPacketId, packetId);
            break;
        case 2:
        /* Update packet */
        rc = prvTgfNetIfSdmaTxGeneratorPacketUpdate(prvTgfDevNum,
                                                    packetId,
                                                    &packetParams,
                                                    packetBuffPtr,
                                                    PRV_TGF_PACKET_LENGTH_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketUpdate: %d, %d",
                                 prvTgfDevNum, packetId);
            break;
        default:
                UTF_VERIFY_EQUAL1_STRING_MAC(0,removeAddOrUpdate,
                                 "Unexpected link list operation: %d",
                                  removeAddOrUpdate);
            break;
    }

    /* Wait so the transition time will not be too short */
#ifdef ASIC_SIMULATION
        cpssOsTimerWkAfter(10);
#else
        cpssOsTimerWkAfter(1);
#endif

    /* disable capture under traffic result in not desirable captured packets
       when it's already disabled. Provide time to avoid such packets. */
    tgfTrafficGeneratorPortTxEthCaptureWaitAfterDisableLoopbackSet(GT_TRUE);

    /* Disable capture Tx */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);

    tgfTrafficGeneratorPortTxEthCaptureWaitAfterDisableLoopbackSet(GT_FALSE);

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

        switch( packetBuff[1] )
        {
            case 0: packetBuffPtr = &packetData0[0];
                    break;
            case 1: packetBuffPtr = &packetData1[0];
                    break;
            case 2: packetBuffPtr = &packetData2[0];
                    break;
            default:
                    UTF_VERIFY_EQUAL1_STRING_MAC(0, packetBuff[1],
                                 "Unexpected packetBuff[1]: %d", packetBuff[1]);
                break;
        }

        /* Packet length check */
        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_PACKET_LENGTH_CNS, packetActualLength,
                                     "packet length different then expected, %d",
                                     callIdentifier);
        /* Packet content length */
        rc = (GT_STATUS)cpssOsMemCmp(
                    (GT_VOID*)packetBuffPtr, (GT_VOID*)packetBuff,
                    packetActualLength - TGF_CRC_LEN_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "packet content different then expected, %d",
                                     callIdentifier);

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
}

/**
* @internal prvTgfNetIfTxSdmaGeneratorTransitionStatePacketsAdd function
* @endinternal
*
* @brief   Adding packets to the queues linked lists.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTransitionStatePacketsAdd
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    packetId;
    GT_U64    rateValue;
    GT_U64    actualRateValue;

    prvTgfNetIfTxSdmaGeneratorTransitionStatePacketParamSet();

    /* AUTODOC: Add 1st packet */
    packetsUpdateArr[0] = packetData0[1];
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData0[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                  prvTgfDevNum, 0);

    UTF_VERIFY_EQUAL2_STRING_MAC(0, packetId,
                                 "different packet id [%d] then expected [%d]",
                                 packetId, 0);

    /* AUTODOC: Add 2nd packet */
    packetsUpdateArr[1] = packetData1[1];
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData1[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                  prvTgfDevNum, 1);

    UTF_VERIFY_EQUAL2_STRING_MAC(1, packetId,
                                 "different packet id [%d] then expected [%d]",
                                 packetId, 1);

    /* AUTODOC: Add 3rd packet */
    packetsUpdateArr[2] = packetData2[1];
    rc = prvTgfNetIfSdmaTxGeneratorPacketAdd(prvTgfDevNum,
                                             &packetParams,
                                             &packetData2[0],
                                             PRV_TGF_PACKET_LENGTH_CNS,
                                             &packetId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorPacketAdd: %d, %d",
                                  prvTgfDevNum, 2);

    UTF_VERIFY_EQUAL2_STRING_MAC(2, packetId,
                                 "different packet id [%d] then expected [%d]",
                                 packetId, 2);

    /* AUTODOC: Set queue rate mode to NO GAP */
    rateValue.l[0] = 0;
    rateValue.l[1] = 0;
    rc = prvTgfNetIfSdmaTxGeneratorRateSet(prvTgfDevNum,
                                           PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                           PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                           rateValue, &actualRateValue);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorRateSet: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                 PRV_TGF_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E,
                                 0);


    /* AUTODOC: enable tail drop by set HOL mode */
    /* AUTODOC: configure Tail Drop to avoid TxQ Descriptor Full interrupts storming */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);

    /* AUTODOC: Enable Tx SDMA generator for queue */
    rc = prvTgfNetIfSdmaTxGeneratorEnable(prvTgfDevNum,
                                          PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                          GT_FALSE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfNetIfSdmaTxGeneratorEnable: %d, %d, %d",
                                 PRV_TGF_TX_SDMA_GENERATOR_Q_CNS,
                                 GT_FALSE, 0);

#ifdef ASIC_SIMULATION
        /* Due to simulation transition from idle/disabled to running/enabled*/
        cpssOsTimerWkAfter(1000);
#endif
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorTransitionStateGenerate function
* @endinternal
*
* @brief   Remove, Add & update managment loop for transit check.
*/
static GT_VOID prvTgfNetIfTxSdmaGeneratorTransitionStateGenerate
(
    GT_VOID
)
{
    GT_U32    repeatFactor;
    GT_U32    ii;
    GT_U32 packetId;
    GT_U32 removeAddOrUpdate;

    /* Repeat factor */
#ifdef ASIC_SIMULATION
        repeatFactor = 3;
#else
        repeatFactor = 100;
#endif

    for( ii = 0; ii < repeatFactor ; ii++ )
    {
        for( packetId = 0 ;
             packetId < PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS ;
             packetId++ )
        {
            for ( removeAddOrUpdate = 0 ;
                  removeAddOrUpdate < 3;
                  removeAddOrUpdate++)
            {
                prvTgfNetIfTxSdmaGeneratorTransitionStateVerify(
                                                            packetId,
                                                            removeAddOrUpdate,
                   (ii*3 + packetId)*PRV_TGF_NUM_OF_PACKETS_IN_Q_CHAIN_CNS +
                                                            removeAddOrUpdate);
            }
        }
    }
}


/**
* @internal prvTgfNetIfTxSdmaGeneratorTransitionStateTest function
* @endinternal
*
* @brief   Tx SDMA generator rate adaptation for flow interval test.
*/
GT_VOID prvTgfNetIfTxSdmaGeneratorTransitionStateTest
(
    GT_VOID
)
{
    prvTgfNetIfTxSdmaGeneratorTransitionStateTestInit();

    prvTgfNetIfTxSdmaGeneratorTransitionStatePacketsAdd();

    prvTgfNetIfTxSdmaGeneratorTransitionStateGenerate();

    prvTgfNetIfTxSdmaGeneratorTransitionStateTestRestore();
}

