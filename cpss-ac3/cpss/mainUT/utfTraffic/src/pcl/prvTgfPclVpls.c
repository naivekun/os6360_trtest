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
* @file prvTgfPclVpls.c
*
* @brief VPLS specific PCL features testing
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfLogicalTargetGen.h>
#include <common/tgfL2MllGen.h>
#include <pcl/prvTgfPclMiscellanous.h>
#include <gtOs/gtOsTimer.h>

/* parameter value to ignore */
#define PRV_TGF_VPLS_IGNORE_VALUE_CNS  0xFFFFFFFF

/* port number to send traffic to */
#define PRV_TGF_VPLS_SEND_PORT_IDX_CNS            1

/* port number to receive traffic from (to check) */
#define PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS         2

/* port number to not receive traffic from (to check) */
#define PRV_TGF_VPLS_NOT_RECEIVE_PORT_IDX_CNS     3

/* VLAN (default, i.e PVID) assigned to packet */
#define PRV_TGF_VPLS_PVID_CNS     5

/* VLAN (FID) assigned to packet */
#define PRV_TGF_VPLS_ASSIGN_FID_CNS     55

/* amount of sent packets */
#define PRV_TGF_VPLS_PACKET_NUM_CNS 2

/* PCL rule index */
static GT_U32   prvTgfPclVplsRuleIndex = 1;

/* TTI rule index */
static GT_U32   prvTgfPclVplsTtiRuleIndex = 5;

/* logical target dev id */
#define PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS  25
#define PRV_TGF_VPLS_LOG_TARGET1_DEV_ID_CNS 26

/* logical target port number */
#define PRV_TGF_VPLS_LOG_TARGET_PORT_NUM_CNS   9
#define PRV_TGF_VPLS_LOG_TARGET1_PORT_NUM_CNS 10

/* policer metering entry index */
#define PRV_TGF_VPLS_POLICER_METER_INDEX_CNS 3

/* policer counter index */
#define PRV_TGF_VPLS_POLICER_COUNTER_INDEX_CNS 60

/* VIDX for L2 MLL */
#define PRV_TGF_VPLS_MLL_VIDX_CNS 999

/* packet Mesh Id (filterd) */
#define PRV_TGF_VPLS_PKT_MESH_ID_CNS 1

/* pass Mesh Id (not filterd) */
#define PRV_TGF_VPLS_PASS_MESH_ID_CNS 2

/* policer metering entry index */
#define PRV_TGF_VPLS_POLICER_METER_INDEX_CNS 3

/* policer counter index */
#define PRV_TGF_VPLS_POLICER_COUNTER_INDEX_CNS 60

/********************************************************************/

/* L2 part of packet unicast */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x11}                 /* saMac */
};

/* ethertype part of ETH_OTHER packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherOtherTypePart = {0x2222};

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};


/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfEthernetPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherOtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfEthernetPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadLongDataArr)),            /* totalLen */
    (sizeof(prvTgfEthernetPacketPartArray)
     / sizeof(TGF_PACKET_PART_STC)),                       /* numOfParts */
    prvTgfEthernetPacketPartArray                          /* partsArray */
};


/* L2 part of packet multicast */
static TGF_PACKET_L2_STC prvTgfMcPacketL2Part =
{
    {0x01, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x11}                 /* saMac */
};

/* PARTS of multicast packet */
static TGF_PACKET_PART_STC prvTgfEthernetMcPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfMcPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherOtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* multicast PACKET to send */
static TGF_PACKET_STC prvTgfEthernetMcPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadLongDataArr)),            /* totalLen */
    (sizeof(prvTgfEthernetMcPacketPartArray)
     / sizeof(TGF_PACKET_PART_STC)),                       /* numOfParts */
    prvTgfEthernetMcPacketPartArray                          /* partsArray */
};



/**
* @internal prvTgfEthAndToCpuPacketSendCountersCheck function
* @endinternal
*
* @brief   Checks and logs port counters and amount packets received by CPU
*
* @param[in] burstCount               - amount of packets sent before this check
* @param[in] checkPortsNum            - amount of ports to check their counters
* @param[in] checkPortIndexArr[]      - array of indexes of checked ports
* @param[in] packetsSentArr[]         - array of expected results:
*                                      GT_TRUE  - all packets sent from the checked port
*                                      GT_FALSE - no  packets sent from the checked port
* @param[in] checkCpuPort             - GT_TRUE  - check CPU port,
*                                      GT_FALSE - don't check CPU port
* @param[in] packetsToCpuSent         - GT_TRUE  - all packets sent to CPU
*                                      GT_FALSE - no  packets sent to CPU
*
* @retval GT_OK                    - on success check results
* @retval GT_FAIL                  - on wrong check results
* @retval other                    - on problems in check algorithm
*/
static GT_STATUS prvTgfEthAndToCpuPacketSendCountersCheck
(
    IN GT_U32  burstCount,
    IN GT_U32  checkPortsNum,
    IN GT_U32  checkPortIndexArr[],
    IN GT_BOOL packetsSentArr[],
    IN GT_BOOL checkCpuPort,
    IN GT_BOOL packetsToCpuSent
)
{
    GT_STATUS                     rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                        i, j;
    GT_U32                        packetsNum;
    GT_U32                        packetsToCpuExpected;
    GT_U32                        packetsToCpuReceived;
    GT_U8                         packetBuf[64] = {0};
    GT_U32                        packetBufLen = sizeof(packetBuf) / sizeof(packetBuf[0]);
    GT_U32                        packetLen;
    GT_U8                         devNum;
    GT_U8                         queueCpu;
    TGF_NET_DSA_STC               rxParams;
    GT_STATUS                     finalRc = GT_OK;

    /* print counters */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfReadPortCountersEth(
            prvTgfDevNum, prvTgfPortsArray[i],
            GT_TRUE, &portCntrs);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfReadPortCountersEth");

        for (j = 0; (j < checkPortsNum); j++)
        {
            if (checkPortIndexArr[j] == i)
            {
                packetsNum = (packetsSentArr[j] == GT_FALSE) ? 0 : burstCount;
                if (packetsNum != portCntrs.goodPktsSent.l[0])
                {
                    PRV_UTF_LOG3_MAC(
                        "wrong send packets number: expected %d received %d, port index %d\n",
                        packetsNum, portCntrs.goodPktsSent.l[0], i);
                    finalRc = GT_FAIL;
                }
                break;
            }
        }
    }

    if (checkCpuPort != GT_FALSE)
    {
        packetsToCpuExpected = (packetsToCpuSent == GT_FALSE) ? 0 : burstCount;

        for (packetsToCpuReceived = 0; (1); packetsToCpuReceived++)
        {
            rc = tgfTrafficGeneratorRxInCpuGet(
                TGF_PACKET_TYPE_REGULAR_E,
                ((packetsToCpuReceived == 0) ? GT_TRUE : GT_FALSE) /*getFirst*/,
                GT_TRUE /*trace*/,
                packetBuf, &packetBufLen, &packetLen,
                &devNum, &queueCpu, &rxParams);
            if (rc == GT_NO_MORE)
            {
                break;
            }
            PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorRxInCpuGet");
        }

        PRV_UTF_LOG1_MAC("Cpu received %d packets\n", packetsToCpuReceived);
        if (packetsToCpuExpected != packetsToCpuReceived)
        {
            PRV_UTF_LOG1_MAC(
                "Expected amount of packets ToCpu is %d\n", packetsToCpuExpected);
            finalRc = GT_FAIL;
        }
    }

    return finalRc;
}

/**
* @internal prvTgfSendBurstWithCapture function
* @endinternal
*
* @brief   Sends the burst of packets capturing the egress port.
*         All the burst must reach the CPU
* @param[in] devNum                   - device number
* @param[in] sendPortNum              - number of port to ingerss the packet
* @param[in] receivePortNum           - number of port to egerss the packet
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
* @param[in] burstCount               - burst Count
*
* @retval GT_OK                    - on success, others at fail
*/
static GT_STATUS prvTgfSendBurstWithCapture
(
    IN GT_U8                devNum,
    IN GT_U32                sendPortNum,
    IN GT_U32                receivePortNum,
    IN TGF_PACKET_STC       *prvTgfPacketInfoPtr,
    IN GT_U32               burstCount
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxModeSet");

    /* set Cpu Device for TRAP to CPU action */
    rc = tgfTrafficGeneratorCpuDeviceSet(devNum);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorCpuDeviceSet");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");

    /* setup receive portInterface for capturing */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = devNum;
    portInterface.devPort.portNum  = receivePortNum;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");


    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        devNum, prvTgfPacketInfoPtr,
        burstCount, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(devNum, sendPortNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

#ifdef ASIC_SIMULATION
    osTimerWkAfter(0);
#endif /*ASIC_SIMULATION*/

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    return rc;
}

/**
* @internal prvTgfCheckCapturedBurstByField function
* @endinternal
*
* @brief   Checks bitmap of packet field triggers for the captured burst
*         Packets sent to CPU not from the egress port capturing ignored.
* @param[in] devNum                   - device number
* @param[in] receivePortNum           - number of port that was captured
*                                      numOfField         - amount of packet fields
*                                      fieldArr           - array of packet fields to check with patterns
*                                      expectedResultBmp  - expected matched fields bitmap
*
* @retval GT_OK                    - on success, others at fail
*/
static GT_STATUS prvTgfCheckCapturedBurstByField
(
    IN GT_U8                devNum,
    IN GT_U32                receivePortNum,
    IN GT_U32               burstCount,
    IN GT_U32               fieldOffset,
    IN GT_U32               fieldLength,
    IN GT_U8                fieldValArr[]
)
{
    GT_STATUS                        rc, rc1 = GT_OK;
    GT_U32                           numTriggersBmp;
    TGF_VFD_INFO_STC                 vfdArray[1];
    CPSS_INTERFACE_INFO_STC          portInterface;
    GT_U32                           expectedResultBmp;

    expectedResultBmp = ((1 << burstCount) - 1);

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = devNum;
    portInterface.devPort.portNum = receivePortNum;

    cpssOsMemSet(&vfdArray, 0, sizeof(vfdArray));

    vfdArray[0].mode       = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset     = fieldOffset;
    vfdArray[0].cycleCount = fieldLength;
    cpssOsMemCpy(vfdArray[0].patternPtr, fieldValArr, fieldLength);

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
        &portInterface, 1, vfdArray, &numTriggersBmp);
    if (rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet");

    if ((rc1 == GT_OK) && (numTriggersBmp != expectedResultBmp))
    {
        rc1 = GT_FAIL;
    }

    return rc1;
}

/**
* @internal prvTgfPclVplsGenTrafficGenerateAndCheckCounters function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsGenTrafficGenerateAndCheckCounters
(
    IN TGF_PACKET_STC  *packetInfoPtr,
    IN GT_U32          burstCount,
    IN GT_BOOL         flushFdb,
    IN GT_U32          checkPortsNum,
    IN GT_U32          checkPortIndexArr[],
    IN GT_BOOL         packetsSentArr[],
    IN GT_BOOL         checkCpuPort,
    IN GT_BOOL         packetsToCpuSent
)
{
    GT_STATUS                     rc = GT_OK;

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxModeSet");

    /* set Cpu Device for TRAP to CPU action */
    rc = tgfTrafficGeneratorCpuDeviceSet(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorCpuDeviceSet");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");

    /* reset counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    if (flushFdb != GT_FALSE)
    {
        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");
    }

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, packetInfoPtr,
        burstCount, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS]);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    rc = prvTgfEthAndToCpuPacketSendCountersCheck(
        burstCount,
        checkPortsNum, checkPortIndexArr, packetsSentArr,
        checkCpuPort, packetsToCpuSent);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthAndToCpuPacketSendCountersCheck");

    return GT_OK;
}

/**
* @internal prvTgfPclVplsTrafficGenerateAndCheckCounters1 function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsTrafficGenerateAndCheckCounters1
(
    IN GT_U32  burstCount,
    IN GT_BOOL packetsSent,
    IN GT_BOOL checkCpuPort,
    IN GT_BOOL packetsToCpuSent
)
{
    GT_U32  checkPortIndexArr[1];
    GT_BOOL packetsSentArr[1];

    checkPortIndexArr[0] = PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS;
    packetsSentArr[0]    = packetsSent;

    return prvTgfPclVplsGenTrafficGenerateAndCheckCounters(
        &prvTgfEthernetPacketInfo,
        burstCount, GT_TRUE /*flushFdb*/, 1 /*checkPortsNum*/,
        checkPortIndexArr, packetsSentArr,
        checkCpuPort, packetsToCpuSent);
}

/**
* @internal prvTgfPclVplsTrafficGenerateAndCheckCounters1Known function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsTrafficGenerateAndCheckCounters1Known
(
    IN GT_U32  burstCount,
    IN GT_BOOL packetsSent,
    IN GT_BOOL checkCpuPort,
    IN GT_BOOL packetsToCpuSent
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    checkPortIndexArr[1];
    GT_BOOL   packetsSentArr[1];

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    checkPortIndexArr[0] = PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS;
    packetsSentArr[0]    = packetsSent;

    rc = prvTgfPclVplsGenTrafficGenerateAndCheckCounters(
        &prvTgfEthernetPacketInfo,
        burstCount, GT_FALSE /*flushFdb*/, 1 /*checkPortsNum*/,
        checkPortIndexArr, packetsSentArr,
        checkCpuPort, packetsToCpuSent);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclVplsGenTrafficGenerateAndCheckCounters");

    return rc1;
}

/**
* @internal prvTgfPclVplsTrafficGenerateAndCheckCounters2Vidx function
* @endinternal
*
* @brief   None
*
* @param[in] burstCount               - burst count
*                                       None
*/
static GT_STATUS prvTgfPclVplsTrafficGenerateAndCheckCounters2Vidx
(
    IN GT_U32  burstCount
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    checkPortIndexArr[2];
    GT_BOOL   packetsSentArr[2];

    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(
        prvTgfMcPacketL2Part.daMac,
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        PRV_TGF_VPLS_MLL_VIDX_CNS,
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    checkPortIndexArr[0] = PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS;
    packetsSentArr[0]    = GT_TRUE;
    checkPortIndexArr[1] = PRV_TGF_VPLS_NOT_RECEIVE_PORT_IDX_CNS;
    packetsSentArr[1]    = GT_FALSE;

    rc = prvTgfPclVplsGenTrafficGenerateAndCheckCounters(
        &prvTgfEthernetMcPacketInfo,
        burstCount, GT_FALSE /*flushFdb*/, 2 /*checkPortsNum*/,
        checkPortIndexArr, packetsSentArr,
        GT_TRUE/*checkCpuPort*/, GT_FALSE/*packetsToCpuSent*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclVplsGenTrafficGenerateAndCheckCounters");

    return rc1;
}

/**
* @internal prvTgfPclVplsTtiEtherTypeRuleActionSet function
* @endinternal
*
* @brief   Set TTI Rule test settings:
*         - Enable specified port for Eth lookup
*         - Set TTI rule action
*         - Set TTI rule that matces all packets
* @param[in] packetCmd                - packet command
* @param[in] unknownSaCmd             - unknown SA command
* @param[in] sendPortNum              - port number to send packet
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfPclVplsTtiEtherTypeRuleActionSet
(
    IN  CPSS_PACKET_CMD_ENT           packetCmd,
    IN  CPSS_PACKET_CMD_ENT           unknownSaCmd,
    IN  GT_U32                         sendPortNum
)
{

    GT_STATUS                  rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC   ttiAction = {0};
    PRV_TGF_TTI_RULE_UNT       ttiPattern;
    PRV_TGF_TTI_RULE_UNT       ttiMask;


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* enable the TTI lookup for Ethernet at the port */
    rc = prvTgfTtiPortLookupEnableSet(sendPortNum, PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* set the TTI Rule Pattern, Mask and Action for Ether Type Key */

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet((GT_VOID*) &ttiPattern,0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,   0, sizeof(ttiMask));

    /* set TTI Action */
    ttiAction.command                       = packetCmd;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;

    ttiAction.redirectCommand               = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.tag1VlanCmd                   = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction.tag1VlanId                    = 1;

    if (unknownSaCmd != CPSS_PACKET_CMD_NONE_E)
    {
        ttiAction.unknownSaCommandEnable = GT_TRUE;
        ttiAction.unknownSaCommand       = unknownSaCmd;
    }

    rc = prvTgfTtiRule2Set(prvTgfPclVplsTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclVplsTtiEtherTypeRuleRestore function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Disable port 0 for Eth lookup
*         - Invalidate TTI rule
* @param[in] sendPortNum              - port number to send packet
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfPclVplsTtiEtherTypeRuleRestore
(
    IN  GT_U32                         sendPortNum
)
{
    GT_STATUS rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Restoring TTI Configuration =======\n");

    /* enable the TTI lookup for Ethernet at the port */
    rc = prvTgfTtiPortLookupEnableSet(sendPortNum, PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfPclVplsTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclVplsPolicerSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclVplsPolicerSet
(
    IN GT_U32  pktConformLevel,
    IN GT_BOOL floodedOnlyMeteringEnable,
    IN GT_BOOL floodedOnlyCountingEnable,
    IN GT_U32  meterEntryIndex,
    IN GT_U32  counterEntryIndex
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_ENTRY_STC           metteringEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    PRV_TGF_POLICER_BILLING_ENTRY_STC   prvTgfBillingCntr;
    PRV_TGF_POLICER_STAGE_TYPE_ENT      policerStage;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntr;

    policerStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        policerStage,
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
        prvTgfDevNum,
        policerStage,
        PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    rc = prvTgfPolicerStageMeterModeSet(
        policerStage,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
        policerStage,
        PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);

    /* set the conformance level as RED or GREEN */
    prvTgfPolicerConformanceLevelForce(pktConformLevel);

    /* enable metering */
    rc = prvTgfPolicerMeteringEnableSet(policerStage, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

    /* create metering entry per SEND Port                 */
    /* all structure members not relevant to DXCH3 omitted */
    cpssOsMemSet(&metteringEntry, 0, sizeof(metteringEntry));
    metteringEntry.meterColorMode     = CPSS_POLICER_COLOR_BLIND_E;
    metteringEntry.qosProfile         = 0;
    metteringEntry.remarkMode         = PRV_TGF_POLICER_REMARK_MODE_L2_E;
    metteringEntry.meterMode          = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;
    metteringEntry.mngCounterSet      = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    metteringEntry.tbParams.srTcmParams.cir = 1;
    metteringEntry.tbParams.srTcmParams.cbs = 0;
    metteringEntry.tbParams.srTcmParams.ebs = 0;
    metteringEntry.countingEntryIndex = 0;
    metteringEntry.modifyUp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    metteringEntry.modifyDscp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    metteringEntry.modifyDp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    metteringEntry.yellowPcktCmd      = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E ;
    metteringEntry.redPcktCmd         = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;

    rc = prvTgfPolicerEntrySet(
        policerStage,
        meterEntryIndex,
        &metteringEntry, &tbParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerEntrySet");

    /* reset Policer Billing Counters */
    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;

    rc = prvTgfPolicerBillingEntrySet(
        prvTgfDevNum, policerStage,
        counterEntryIndex,
        &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerBillingEntrySet");

    /* reset Policer Management Counters */
    cpssOsMemSet(&prvTgfMngCntr, 0, sizeof(prvTgfMngCntr));

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet");

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet");

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet");

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet");

    /* Flush counters */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(policerStage);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingWriteBackCacheFlush");

    rc = prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet(floodedOnlyMeteringEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet");

    rc = prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet(floodedOnlyCountingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet");

};

/**
* @internal prvTgfPclVplsPolicerRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsPolicerRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    PRV_TGF_POLICER_STAGE_TYPE_ENT      policerStage;

    policerStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    rc = prvTgfPolicerCountingModeSet(
        prvTgfDevNum,
        policerStage,
        PRV_TGF_POLICER_COUNTING_DISABLE_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerCountingModeSet");

    rc = prvTgfPolicerStageMeterModeSet(
        policerStage,
        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerStageMeterModeSet");

    /* set the conformance level as GREEN */
    prvTgfPolicerConformanceLevelForce(0);

    /* disable metering */
    rc = prvTgfPolicerMeteringEnableSet(policerStage, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerMeteringEnableSet");

    rc = prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerFlowBasedMeteringToFloodedOnlyEnableSet");

    rc = prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet");

    return rc1;
};

/**
* @internal prvTgfPclVplsL2MllSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsL2MllSet
(
    IN GT_VOID
)
{
    GT_STATUS                    rc, rc1 = GT_OK;
    PRV_TGF_L2_MLL_PAIR_STC      l2Mll;

    PRV_TGF_LOGICAL_TARGET_MAPPING_STC  logicalTargetMappingEntry;

    /* Logical Target Mapping enable */
    rc = prvTgfLogicalTargetMappingEnableSet(prvTgfDevNum, GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingEnableSet");

    /* enable device mapping */
    rc = prvTgfLogicalTargetMappingDeviceEnableSet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS,
        GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingDeviceEnableSet");

    /* enable device mapping */
    rc = prvTgfLogicalTargetMappingDeviceEnableSet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET1_DEV_ID_CNS,
        GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingDeviceEnableSet");

    /* mapping logical target to interface */
    cpssOsMemSet(&logicalTargetMappingEntry, 0, sizeof(logicalTargetMappingEntry));
    logicalTargetMappingEntry.outputInterface.isTunnelStart = GT_FALSE;
    logicalTargetMappingEntry.outputInterface.physicalInterface.type = CPSS_INTERFACE_PORT_E;
    logicalTargetMappingEntry.outputInterface.physicalInterface.devPort.hwDevNum = prvTgfDevNum;
    logicalTargetMappingEntry.outputInterface.physicalInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS];

    rc = prvTgfLogicalTargetMappingTableEntrySet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS,
        PRV_TGF_VPLS_LOG_TARGET_PORT_NUM_CNS,
        &logicalTargetMappingEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingTableEntrySet");

    /* mapping logical target to interface */
    cpssOsMemSet(&logicalTargetMappingEntry, 0, sizeof(logicalTargetMappingEntry));
    logicalTargetMappingEntry.outputInterface.isTunnelStart = GT_FALSE;
    logicalTargetMappingEntry.outputInterface.physicalInterface.type = CPSS_INTERFACE_PORT_E;
    logicalTargetMappingEntry.outputInterface.physicalInterface.devPort.hwDevNum = prvTgfDevNum;
    logicalTargetMappingEntry.outputInterface.physicalInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_VPLS_NOT_RECEIVE_PORT_IDX_CNS];

    rc = prvTgfLogicalTargetMappingTableEntrySet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET1_DEV_ID_CNS,
        PRV_TGF_VPLS_LOG_TARGET1_PORT_NUM_CNS,
        &logicalTargetMappingEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingTableEntrySet");

    cpssOsMemSet(&l2Mll, 0, sizeof(l2Mll));

    l2Mll.firstMllNode.meshId = PRV_TGF_VPLS_PASS_MESH_ID_CNS;
    l2Mll.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    l2Mll.firstMllNode.egressInterface.devPort.hwDevNum =
        PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS;
    l2Mll.firstMllNode.egressInterface.devPort.portNum =
        PRV_TGF_VPLS_LOG_TARGET_PORT_NUM_CNS;
    l2Mll.firstMllNode.last = GT_FALSE;

    l2Mll.secondMllNode.meshId = PRV_TGF_VPLS_PKT_MESH_ID_CNS;
    l2Mll.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    l2Mll.secondMllNode.egressInterface.devPort.hwDevNum =
        PRV_TGF_VPLS_LOG_TARGET1_DEV_ID_CNS;
    l2Mll.secondMllNode.egressInterface.devPort.portNum =
        PRV_TGF_VPLS_LOG_TARGET1_PORT_NUM_CNS;
    l2Mll.secondMllNode.last = GT_TRUE;

    l2Mll.nextPointer = 0;

    rc = prvTgfL2MllPairWrite(
        prvTgfDevNum,
        PRV_TGF_VPLS_MLL_VIDX_CNS /*mllPairEntryIndex*/,
        PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
        &l2Mll);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfL2MllPairWrite");

    rc = prvTgfL2MllVidxEnableSet(
        prvTgfDevNum, PRV_TGF_VPLS_MLL_VIDX_CNS, GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPolicerFlowBasedBillingToFloodedOnlyEnableSet");

    return rc1;
}

/**
* @internal prvTgfPclVplsL2MllRestore function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclVplsL2MllRestore
(
    IN GT_VOID
)
{
    GT_STATUS                    rc = GT_OK;
    PRV_TGF_L2_MLL_PAIR_STC l2Mll;

    cpssOsMemSet(&l2Mll, 0, sizeof(l2Mll));

    rc = prvTgfL2MllPairWrite(
        prvTgfDevNum,
        PRV_TGF_VPLS_MLL_VIDX_CNS /*mllPairEntryIndex*/,
        PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
        &l2Mll);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: %d", prvTgfDevNum);

    rc = prvTgfL2MllVidxEnableSet(
        prvTgfDevNum, PRV_TGF_VPLS_MLL_VIDX_CNS, GT_FALSE /*enable*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllVidxEnableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclVplsConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* disable ingress policy on ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclVplsRuleIndex, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* enables ingress policy */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxStartCapture");

    /* reset counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(1);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VPLS_PVID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VPLS_ASSIGN_FID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* restore TTI configuration */
    prvTgfPclVplsTtiEtherTypeRuleRestore(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS]);

    /* disable device mapping */
    rc = prvTgfLogicalTargetMappingDeviceEnableSet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingDeviceEnableSet");

    rc = prvTgfLogicalTargetMappingDeviceEnableSet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET1_DEV_ID_CNS,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingDeviceEnableSet");

    /* Logical Target Mapping disable */
    rc = prvTgfLogicalTargetMappingEnableSet(prvTgfDevNum, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfLogicalTargetMappingEnableSet");

    prvTgfPclVplsPolicerRestore();

    prvTgfPclVplsL2MllRestore();

    return rc1;
};

/**
* @internal prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign
(
    IN    GT_U8                       virtSrcPortDev,
    IN    GT_U8                       virtSrcPortNum,
    IN    GT_BOOL                     userTagAcEnable,
    IN    GT_BOOL                     meshIdSetEnable,
    IN    GT_U8                       meshId,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *maskPtr,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *pattPtr,
    INOUT PRV_TGF_PCL_ACTION_STC      *actionPtr
)
{
    GT_STATUS      rc = GT_OK;

    /* mask for MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));

    /* difine mask, pattern and action */
    cpssOsMemSet(pattPtr, 0, sizeof(*pattPtr));

    /* action settings */
    cpssOsMemSet(actionPtr, 0, sizeof(*actionPtr));

    /* action redirect */
    actionPtr->pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->bypassIngressPipe                                = GT_FALSE;
    actionPtr->redirect.redirectCmd                             =
        PRV_TGF_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E;
    actionPtr->redirect.data.logicalSourceInterface.logicalInterface.type            = CPSS_INTERFACE_PORT_E;
    actionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum  = virtSrcPortDev;
    actionPtr->redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum = virtSrcPortNum;

    actionPtr->redirect.data.logicalSourceInterface.userTagAcEnable = userTagAcEnable;
    actionPtr->redirect.data.logicalSourceInterface.sourceMeshIdSetEnable = meshIdSetEnable;
    actionPtr->redirect.data.logicalSourceInterface.sourceMeshId = meshId;

    /* FID assignment */
    actionPtr->vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    actionPtr->vlan.vlanId     = PRV_TGF_VPLS_ASSIGN_FID_CNS;
    actionPtr->vlan.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    return rc;
};

/**
* @internal prvTgfPclVplsTrafficPclRuleUnknownSaCmd function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsTrafficPclRuleUnknownSaCmd
(
    IN    CPSS_PACKET_CMD_ENT            unknownSaCmd,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT    *maskPtr,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT    *pattPtr,
    INOUT PRV_TGF_PCL_ACTION_STC         *actionPtr
)
{
    GT_STATUS      rc = GT_OK;

    /* mask for MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));

    /* difine mask, pattern and action */
    cpssOsMemSet(pattPtr, 0, sizeof(*pattPtr));

    /* action settings */
    cpssOsMemSet(actionPtr, 0, sizeof(*actionPtr));

    actionPtr->pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->bypassIngressPipe                                = GT_FALSE;

    /* FID assignment */
    actionPtr->vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    actionPtr->vlan.vlanId     = PRV_TGF_VPLS_ASSIGN_FID_CNS;
    actionPtr->vlan.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* Unknown Sa Cmd */
    if (unknownSaCmd != CPSS_PACKET_CMD_NONE_E)
    {
        actionPtr->unknownSaCommandEnable = GT_TRUE;
        actionPtr->unknownSaCommand = unknownSaCmd;
    }

    return rc;
};

/**
* @internal prvTgfPclVplsTrafficPclRulePolicerEntrySet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclVplsTrafficPclRulePolicerEntrySet
(
    IN    GT_BOOL                        meterEnable,
    IN    GT_BOOL                        counterEnable,
    IN    GT_U32                         policerIndex,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT    *maskPtr,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT    *pattPtr,
    INOUT PRV_TGF_PCL_ACTION_STC         *actionPtr
)
{
    GT_STATUS      rc = GT_OK;

    /* mask for MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));

    /* difine mask, pattern and action */
    cpssOsMemSet(pattPtr, 0, sizeof(*pattPtr));

    /* action settings */
    cpssOsMemSet(actionPtr, 0, sizeof(*actionPtr));

    actionPtr->pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->bypassIngressPipe    = GT_FALSE;

    /* FID assignment */
    actionPtr->vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    actionPtr->vlan.vlanId     = PRV_TGF_VPLS_ASSIGN_FID_CNS;
    actionPtr->vlan.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* Policer Cfg */
    switch ((BOOL2BIT_MAC(meterEnable) << 1) | BOOL2BIT_MAC(counterEnable))
    {
        case 0:
        actionPtr->policer.policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
            break;
        case 1:
            actionPtr->policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            break;
        case 2:
            actionPtr->policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_METER_ONLY_E;
            break;
        case 3:
            actionPtr->policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
            break;
        default: return GT_BAD_PARAM;
    }
    actionPtr->policer.policerId = policerIndex;
    return rc;
};

/**
* @internal prvTgfPclVplsVirtSrcPortAssign_CfgAndTraffic function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclVplsVirtSrcPortAssign_CfgAndTraffic
(
    IN    GT_U8                       virtSrcPortDev,
    IN    GT_U8                       virtSrcPortNum
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;
    PRV_TGF_MAC_ENTRY_KEY_STC    macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;

    /* create VLAN entry contains all ports untagged */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    rc = prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign(
        virtSrcPortDev, virtSrcPortNum, GT_FALSE /*userTagAcEnable*/,
        GT_FALSE /*meshIdEnable*/, 0 /*meshId*/,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign");

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        prvTgfPclVplsRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclVplsRuleIndex);

    rc = prvTgfPclVplsTrafficGenerateAndCheckCounters1(
        PRV_TGF_VPLS_PACKET_NUM_CNS/*burstCount*/,
        GT_TRUE /*packetsSent*/,
        GT_TRUE /*checkCpuPort*/,
        GT_FALSE /*packetsToCpuSent*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficGenerateAndCheckCounters1");

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
    macEntryKey.entryType          = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VPLS_ASSIGN_FID_CNS;
    cpssOsMemCpy(
        &(macEntryKey.key.macVlan.macAddr.arEther[0]),
        &(prvTgfPacketL2Part.saMac[0]), 6);

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntryGet");

    PRV_UTF_LOG3_MAC(
        "FDB Entry: VID: %d, portDev: %d, portNum: %d\n",
        macEntry.key.key.macVlan.vlanId,
        macEntry.dstInterface.devPort.hwDevNum,
        macEntry.dstInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        macEntry.key.key.macVlan.vlanId,
        "wrong FDB Entry VID");

    UTF_VERIFY_EQUAL0_STRING_MAC(
        virtSrcPortDev,
        macEntry.dstInterface.devPort.hwDevNum,
        "wrong FDB Entry hwDevNum");

    UTF_VERIFY_EQUAL0_STRING_MAC(
        virtSrcPortNum,
        macEntry.dstInterface.devPort.portNum,
        "wrong FDB Entry portNum");
}

/**
* @internal prvTgfPclVplsUnknownSaCmd_CfgAndTraffic function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclVplsUnknownSaCmd_CfgAndTraffic
(
    IN    CPSS_PACKET_CMD_ENT   unknownSaCmd
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;
    GT_BOOL                     packetsSent;
    GT_BOOL                     packetsToCpuSent;

    /* create VLAN entry contains all ports untagged */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VPLS_ASSIGN_FID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite");

    rc = prvTgfPclVplsTrafficPclRuleUnknownSaCmd(
        unknownSaCmd,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficPclRuleUnknownSaCmd");

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        prvTgfPclVplsRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclVplsRuleIndex);

    switch (unknownSaCmd)
    {
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            packetsSent      = GT_FALSE;
            packetsToCpuSent = GT_TRUE;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            packetsSent      = GT_TRUE;
            packetsToCpuSent = GT_TRUE;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            packetsSent      = GT_FALSE;
            packetsToCpuSent = GT_FALSE;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            /* TTI caused mirror to CPU and PCL Soft Drop is TRAP */
            packetsSent      = GT_FALSE;
            packetsToCpuSent = GT_TRUE;
            prvTgfPclVplsTtiEtherTypeRuleActionSet(
                CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                CPSS_PACKET_CMD_NONE_E,
                prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS]);
            break;
        case CPSS_PACKET_CMD_FORWARD_E:
            /* TTI caused hard drop overriden by PCL Forward */
            packetsSent      = GT_TRUE;
            packetsToCpuSent = GT_FALSE;
            prvTgfPclVplsTtiEtherTypeRuleActionSet(
                CPSS_PACKET_CMD_FORWARD_E,
                CPSS_PACKET_CMD_DROP_HARD_E,
                prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS]);
            break;
        case CPSS_PACKET_CMD_NONE_E:
            /* TTI caused TRAP to CPU as is */
            packetsSent      = GT_FALSE;
            packetsToCpuSent = GT_TRUE;
            prvTgfPclVplsTtiEtherTypeRuleActionSet(
                CPSS_PACKET_CMD_FORWARD_E,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS]);
            break;
        default:
            PRV_UTF_LOG1_MAC("Test does not support command: %d\n", unknownSaCmd);
            /* must never occur - treat as TRAP */
            packetsSent      = GT_FALSE;
            packetsToCpuSent = GT_TRUE;
            break;
    }

    /* one packet only: the second's SA will already be learned (and so known) */
    rc = prvTgfPclVplsTrafficGenerateAndCheckCounters1(
        1 /*burstCount*/, packetsSent,
        GT_TRUE /*checkCpuPort*/, packetsToCpuSent);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficGenerateAndCheckCounters1");
}

/**
* @internal prvTgfPclVplsVirtSrcPortAndAcTagEnableAssign_CfgAndTraffic function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclVplsVirtSrcPortAndAcTagEnableAssign_CfgAndTraffic
(
    IN    GT_U8                       virtSrcPortDev,
    IN    GT_U8                       virtSrcPortNum,
    IN    GT_BOOL                     userTagAcEnable
)
{
    GT_STATUS                    rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT  mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT  pattern;
    PRV_TGF_PCL_ACTION_STC       action;
    GT_U16                       vid;
    GT_U8                        vidLowByte;
    PRV_TGF_LOGICAL_TARGET_MAPPING_STC logicalTargetMappingEntry;

    vid =
        (userTagAcEnable == GT_FALSE)
            ? PRV_TGF_VPLS_ASSIGN_FID_CNS
            : PRV_TGF_VPLS_PVID_CNS;

    /* Logical Target Mapping enable */
    rc = prvTgfLogicalTargetMappingEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLogicalTargetMappingEnableSet");

    /* enable device mapping */
    rc = prvTgfLogicalTargetMappingDeviceEnableSet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS,
        GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLogicalTargetMappingDeviceEnableSet");

    /* mapping logical target to interface */
    cpssOsMemSet(&logicalTargetMappingEntry, 0, sizeof(logicalTargetMappingEntry));
    logicalTargetMappingEntry.outputInterface.isTunnelStart = GT_FALSE;
    logicalTargetMappingEntry.outputInterface.physicalInterface.type = CPSS_INTERFACE_PORT_E;
    logicalTargetMappingEntry.outputInterface.physicalInterface.devPort.hwDevNum = prvTgfDevNum;
    logicalTargetMappingEntry.outputInterface.physicalInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS];

    rc = prvTgfLogicalTargetMappingTableEntrySet(
        prvTgfDevNum,
        PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS,
        PRV_TGF_VPLS_LOG_TARGET_PORT_NUM_CNS,
        &logicalTargetMappingEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfLogicalTargetMappingTableEntrySet");

    /* create VLAN entry contains all ports tagged */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* create VLAN entry contains all ports tagged */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VPLS_PVID_CNS,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* set default vlanId */
    rc = prvTgfBrgVlanPvidSet(PRV_TGF_VPLS_PVID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPvidSet");

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        PRV_TGF_VPLS_LOG_TARGET_DEV_ID_CNS,
        PRV_TGF_VPLS_LOG_TARGET_PORT_NUM_CNS,
        GT_TRUE /*isStatic*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    vidLowByte = (GT_U8)(vid & 0xFF);

    rc = prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign(
        virtSrcPortDev, virtSrcPortNum, userTagAcEnable,
        GT_FALSE /*meshIdEnable*/, 0 /*meshId*/,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign");

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        prvTgfPclVplsRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclVplsRuleIndex);

    prvTgfSendBurstWithCapture(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS],
        &prvTgfEthernetPacketInfo,
        PRV_TGF_VPLS_PACKET_NUM_CNS /*burstCount*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSendBurstWithCapture");

    rc = prvTgfCheckCapturedBurstByField(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_VPLS_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_VPLS_PACKET_NUM_CNS /*burstCount*/,
        15 /*fieldOffset*/,
        1 /*fieldLength*/,
        &vidLowByte);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCheckCapturedBurstByField");

}

/**
* @internal prvTgfPclVplsMeteringCountingToFloodedOnly_CfgAndTraffic function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclVplsMeteringCountingToFloodedOnly_CfgAndTraffic
(
    IN GT_BOOL testMetering, /* otherwise test counting */
    IN GT_BOOL floodedOnlyMeteringEnable,
    IN GT_BOOL floodedOnlyCountingEnable,
    IN GT_U32  meterEntryIndex,
    IN GT_U32  counterEntryIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    PRV_TGF_POLICER_BILLING_ENTRY_STC   prvTgfBillingCntr;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntr;
    GT_U32                              cntPacketsExpected;
    GT_U32                              pktConformLevel;

    /* RED for metering, GREEN for counting */
    pktConformLevel = (testMetering == GT_FALSE) ? 0 : 2;

    prvTgfPclVplsPolicerSet(
        pktConformLevel,
        floodedOnlyMeteringEnable, floodedOnlyCountingEnable,
        meterEntryIndex, counterEntryIndex);

    /* create VLAN entry contains all ports untagged */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    rc = prvTgfPclVplsTrafficPclRulePolicerEntrySet(
        testMetering,
        ((testMetering == GT_FALSE) ? GT_TRUE : GT_FALSE) /*countingEnable*/,
        ((testMetering == GT_FALSE) ? counterEntryIndex : meterEntryIndex),
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficPclRulePolicerEntrySet");

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        prvTgfPclVplsRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclVplsRuleIndex);

    rc = prvTgfPclVplsTrafficGenerateAndCheckCounters1Known(
        PRV_TGF_VPLS_PACKET_NUM_CNS/*burstCount*/,
        GT_TRUE /*packetsSent*/,
        GT_TRUE /*checkCpuPort*/,
        GT_FALSE /*packetsToCpuSent*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficGenerateAndCheckCounters1");

    if (testMetering != GT_FALSE)
    {
        /* get Policer Management Counters */
        rc = prvTgfPolicerManagementCountersGet(
            prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,
            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
            PRV_TGF_POLICER_MNG_CNTR_RED_E,
            &prvTgfMngCntr);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgf3PolicerManagementCountersGet");

        PRV_UTF_LOG1_MAC(
            "*** RED management counter: %d ***\n",
            prvTgfMngCntr.packetMngCntr);

        cntPacketsExpected =
            (floodedOnlyMeteringEnable == GT_FALSE)
                ? PRV_TGF_VPLS_PACKET_NUM_CNS : 0;
        UTF_VERIFY_EQUAL1_STRING_MAC(
            cntPacketsExpected, prvTgfMngCntr.packetMngCntr,
            "MenagementCntr_Red_packet = %d",
            prvTgfMngCntr.packetMngCntr);
    }
    else
    {
        rc = prvTgfPolicerBillingEntryGet(
            prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E,
            counterEntryIndex,
            GT_TRUE /*reset*/,
            &prvTgfBillingCntr);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPolicerBillingEntrySet");

        PRV_UTF_LOG3_MAC(
            "*** billing counter: green %d yellow %d red %d ***\n",
            prvTgfBillingCntr.greenCntr.l[0],
            prvTgfBillingCntr.yellowCntr.l[0],
            prvTgfBillingCntr.redCntr.l[0]);

        cntPacketsExpected =
            (floodedOnlyCountingEnable == GT_FALSE)
                ? PRV_TGF_VPLS_PACKET_NUM_CNS : 0;
        UTF_VERIFY_EQUAL0_STRING_MAC(
            cntPacketsExpected,
            prvTgfBillingCntr.greenCntr.l[0],
            "wrong billing greenCntr");
    }

    return;
}

/**
* @internal prvTgfPclVplsMeshIdAssign_CfgAndTraffic function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclVplsMeshIdAssign_CfgAndTraffic
(
    IN    GT_VOID
)
{
    GT_STATUS                    rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT  mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT  pattern;
    PRV_TGF_PCL_ACTION_STC       action;

    /* create VLAN entry contains all ports tagged */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VPLS_ASSIGN_FID_CNS,
        PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_VPLS_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    rc = prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign(
        25 /*virtSrcPortDev*/,
        51 /*virtSrcPortNum*/,
        GT_FALSE /*userTagAcEnable*/,
        GT_TRUE /*meshIdEnable*/, PRV_TGF_VPLS_PKT_MESH_ID_CNS /*meshId*/,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficPclRuleAndVirtSrcPortAssign");

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        prvTgfPclVplsRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclVplsRuleIndex);

    rc = prvTgfPclVplsL2MllSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsL2MllSet");

    rc = prvTgfPclVplsTrafficGenerateAndCheckCounters2Vidx(
        PRV_TGF_VPLS_PACKET_NUM_CNS /*burstCount*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclVplsTrafficGenerateAndCheckCounters2Vidx");


}

/**
* @internal prvTgfPclVplsVirtSrcPortAssign function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsVirtSrcPortAssign
(
    GT_VOID
)
{
    prvTgfPclVplsVirtSrcPortAssign_CfgAndTraffic(1, 2);

    prvTgfPclVplsConfigurationRestore();

    prvTgfPclVplsVirtSrcPortAssign_CfgAndTraffic(31, 59);

    prvTgfPclVplsConfigurationRestore();

    prvTgfPclVplsVirtSrcPortAssign_CfgAndTraffic(22, 33);

    prvTgfPclVplsConfigurationRestore();
}

/**
* @internal prvTgfPclVplsUnknownSaCmd function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsUnknownSaCmd
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("===== CPSS_PACKET_CMD_TRAP_TO_CPU_E =====\n");
    prvTgfPclVplsUnknownSaCmd_CfgAndTraffic(CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    prvTgfPclVplsConfigurationRestore();

    PRV_UTF_LOG0_MAC("===== CPSS_PACKET_CMD_MIRROR_TO_CPU_E =====\n");
    prvTgfPclVplsUnknownSaCmd_CfgAndTraffic(CPSS_PACKET_CMD_MIRROR_TO_CPU_E);

    prvTgfPclVplsConfigurationRestore();

    PRV_UTF_LOG0_MAC("===== CPSS_PACKET_CMD_DROP_HARD_E =====\n");
    prvTgfPclVplsUnknownSaCmd_CfgAndTraffic(CPSS_PACKET_CMD_DROP_HARD_E);

    prvTgfPclVplsConfigurationRestore();
    PRV_UTF_LOG0_MAC("===== CPSS_PACKET_CMD_DROP_SOFT_E =====\n");
    prvTgfPclVplsUnknownSaCmd_CfgAndTraffic(CPSS_PACKET_CMD_DROP_SOFT_E);

    prvTgfPclVplsConfigurationRestore();

    PRV_UTF_LOG0_MAC("===== CPSS_PACKET_CMD_FORWARD_E =====\n");
    prvTgfPclVplsUnknownSaCmd_CfgAndTraffic(CPSS_PACKET_CMD_FORWARD_E);

    prvTgfPclVplsConfigurationRestore();

    PRV_UTF_LOG0_MAC("===== CPSS_PACKET_CMD_NONE_E =====\n");
    prvTgfPclVplsUnknownSaCmd_CfgAndTraffic(CPSS_PACKET_CMD_NONE_E);

    prvTgfPclVplsConfigurationRestore();

}

/**
* @internal prvTgfPclVplsVirtSrcPortAssignAndAcEnable function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsVirtSrcPortAssignAndAcEnable
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("\n\n\n ***** Keeping default VID in TAG0 *****\n\n\n");

    prvTgfPclVplsVirtSrcPortAndAcTagEnableAssign_CfgAndTraffic(1, 2, GT_TRUE);

    prvTgfPclVplsConfigurationRestore();

    PRV_UTF_LOG0_MAC("\n\n\n ***** setting assigned FID in TAG0 *****\n\n\n");

    prvTgfPclVplsVirtSrcPortAndAcTagEnableAssign_CfgAndTraffic(25, 44, GT_FALSE);

    prvTgfPclVplsConfigurationRestore();
}

/**
* @internal prvTgfPclVplsMeteringToFloodedOnlyEnable function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsMeteringToFloodedOnlyEnable
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("\n\n\n ***** Metering Flooded only, Known DA-packet  *****\n\n\n");

    prvTgfPclVplsMeteringCountingToFloodedOnly_CfgAndTraffic(
        GT_TRUE /*testMetering*/,
        GT_TRUE /*floodedOnlyMeteringEnable*/,
        GT_FALSE /*floodedOnlyCountingEnable*/,
        PRV_TGF_VPLS_POLICER_METER_INDEX_CNS  /*meterEntryIndex*/,
        PRV_TGF_VPLS_POLICER_COUNTER_INDEX_CNS  /*counterEntryIndex*/);

    prvTgfPclVplsConfigurationRestore();

    PRV_UTF_LOG0_MAC("\n\n\n ***** Metering any packets, Known DA-packet  *****\n\n\n");

    prvTgfPclVplsMeteringCountingToFloodedOnly_CfgAndTraffic(
        GT_TRUE /*testMetering*/,
        GT_FALSE /*floodedOnlyMeteringEnable*/,
        GT_FALSE /*floodedOnlyCountingEnable*/,
        PRV_TGF_VPLS_POLICER_METER_INDEX_CNS  /*meterEntryIndex*/,
        PRV_TGF_VPLS_POLICER_COUNTER_INDEX_CNS  /*counterEntryIndex*/);

    prvTgfPclVplsConfigurationRestore();
}

/**
* @internal prvTgfPclVplsBillingToFloodedOnlyEnable function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsBillingToFloodedOnlyEnable
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("\n\n\n ***** Counting Flooded only, Known DA-packet  *****\n\n\n");

    prvTgfPclVplsMeteringCountingToFloodedOnly_CfgAndTraffic(
        GT_FALSE /*testMetering*/,
        GT_FALSE /*floodedOnlyMeteringEnable*/,
        GT_TRUE  /*floodedOnlyCountingEnable*/,
        PRV_TGF_VPLS_POLICER_METER_INDEX_CNS  /*meterEntryIndex*/,
        PRV_TGF_VPLS_POLICER_COUNTER_INDEX_CNS  /*counterEntryIndex*/);

    prvTgfPclVplsConfigurationRestore();

    PRV_UTF_LOG0_MAC("\n\n\n ***** Counting any packets, Known DA-packet  *****\n\n\n");

    prvTgfPclVplsMeteringCountingToFloodedOnly_CfgAndTraffic(
        GT_FALSE /*testMetering*/,
        GT_FALSE /*floodedOnlyMeteringEnable*/,
        GT_FALSE /*floodedOnlyCountingEnable*/,
        PRV_TGF_VPLS_POLICER_METER_INDEX_CNS  /*meterEntryIndex*/,
        PRV_TGF_VPLS_POLICER_COUNTER_INDEX_CNS  /*counterEntryIndex*/);

    prvTgfPclVplsConfigurationRestore();
}

/**
* @internal prvTgfPclVplsVirtualSrcPortMeshIdAssign function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfPclVplsVirtualSrcPortMeshIdAssign
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("\n\n\n ***** prvTgfPclVplsVirtualSrcPortMeshIdAssign  *****\n\n\n");

    prvTgfPclVplsMeshIdAssign_CfgAndTraffic();

    prvTgfPclVplsConfigurationRestore();
}




