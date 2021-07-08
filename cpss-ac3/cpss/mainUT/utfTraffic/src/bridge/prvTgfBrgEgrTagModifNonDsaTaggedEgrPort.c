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
* @file prvTgfBrgEgrTagModifNonDsaTaggedEgrPort.c
*
* @brief Egress Tag Modification on Non-DSA-Tagged Egress Port
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgEgrTagModifNonDsaTaggedEgrPort.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/*  VLAN Id */
#define PRV_TGF_VLANID_2_CNS                    2

/*  number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               1

/* capturing port number 1 */
#define PRV_TGF_CAPTURE_PORT1_IDX_CNS                          2

/* capturing port number 2 */
#define PRV_TGF_CAPTURE_PORT2_IDX_CNS                          3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* tagging array */
static GT_U8   prvTgfTagArray[] = {1, 1, 1, 0};

/* send ports array */
static GT_U8 prvTgfSendPort[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_SEND_PORT_IDX_CNS, PRV_TGF_SEND_PORT_IDX_CNS,
        PRV_TGF_SEND_PORT_IDX_CNS, PRV_TGF_SEND_PORT_IDX_CNS
};

/* capture ports array for the tests */
static GT_U8 prvTgfCapturePortsArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
        PRV_TGF_CAPTURE_PORT1_IDX_CNS, PRV_TGF_CAPTURE_PORT2_IDX_CNS,
        PRV_TGF_CAPTURE_PORT1_IDX_CNS, PRV_TGF_CAPTURE_PORT2_IDX_CNS
};

/* array of destination MACs for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03}
};

/* expected number of Triggers on portIdx 2, 3 */
static GT_U8 prvTgfPacketsCountTriggerArr[][PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    {1, 0, 1, 0},
        {0, 0, 0, 0}
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 1, 1, 0},
    {0, 1, 0, 1},
    {0, 1, 1, 0},
        {0, 1, 0, 1}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 1, 1, 0},
    {0, 1, 0, 1},
    {0, 1, 1, 0},
        {0, 1, 0, 1}
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfUntaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of UNTAGGED packet */
#define PRV_TGF_UNTAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* UNTAGGED PACKET to send */
static TGF_PACKET_STC prvTgfUntaggedPacketInfo =
{
    PRV_TGF_UNTAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfUntaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfUntaggedPacketPartArray                                        /* partsArray */
};

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/* type of traffic to send (tagged or untagged) */
static TGF_PACKET_STC* prvTgfPacketInfoArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    &prvTgfTaggedPacketInfo, &prvTgfTaggedPacketInfo,
        &prvTgfUntaggedPacketInfo, &prvTgfUntaggedPacketInfo
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with tagged ports 0, 1, 2, untagged port 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
                                           prvTgfPortsArray, NULL,
                                           prvTgfTagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: enable FDB Learning, Scanning and Refresh */
        rc = prvTgfBrgFdbLsrEnableSet(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbLsrEnableSet");

    /* AUTODOC: set Independent VLAN Learning lookup mode */
    rc = prvTgfBrgFdbMacVlanLookupModeSet(CPSS_IVL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacVlanLookupModeSet");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfDaMacArr[0],
                                          PRV_TGF_VLANID_2_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CAPTURE_PORT1_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:03, VLAN 2, port 3 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfDaMacArr[1],
                                          PRV_TGF_VLANID_2_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CAPTURE_PORT2_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: set PVID to 2 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[1],
                                 PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[1],
                                 PRV_TGF_VLANID_2_CNS);
}

/**
* @internal prvTgfBrgEgrTagModifNonDsaTaggedEgrPortTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgEgrTagModifNonDsaTaggedEgrPortTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc             = GT_OK;
    GT_U32                      portIter       = 0;
    GT_U32                      packetLen      = 0;
    GT_U32                      sendIter       = 0;
    GT_U32                      numTriggers    = 0;
    GT_U32                                                expectTriggers = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        PRV_UTF_LOG0_MAC("======= Sending packets =======\n\n");

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

                /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* enable capture  */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfCapturePortsArr[sendIter]];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

        /* AUTODOC: configure 2 tagged packets to send: */
        /* AUTODOC:   1 packet - DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=2 */
        /* AUTODOC:   2 packet - DA=00:00:00:00:00:03, SA=00:00:00:00:00:01, VID=2 */
        /* AUTODOC: configure 2 untagged packets to send: */
        /* AUTODOC:   1 packet - DA=00:00:00:00:00:02, SA=00:00:00:00:00:01 */
        /* AUTODOC:   2 packet - DA=00:00:00:00:00:03, SA=00:00:00:00:00:01 */

        /* set destination MAC address in Packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[sendIter],
                     sizeof(prvTgfPacketL2Part.daMac));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter], prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

        /* AUTODOC: send configured packets on port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* check ETH counter */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* calculate packet length */
            if(prvTgfPacketInfoArr[sendIter] == &prvTgfTaggedPacketInfo && /* if packet is tagged and */
                            0 == prvTgfTagArray[portIter] && portIter != prvTgfSendPort[portIter]) /* port is untagged and not equal send port*/
                        {
                                packetLen = prvTgfPacketInfoArr[sendIter]->totalLen - TGF_VLAN_TAG_SIZE_CNS;
            }
            else
            {
                                if(prvTgfPacketInfoArr[sendIter] == &prvTgfUntaggedPacketInfo && /* if packet is untagged and */
                                   1 == prvTgfTagArray[portIter] && portIter != prvTgfSendPort[portIter]) /* port is untagged and not equal send port*/
                                {
                                        packetLen = prvTgfPacketInfoArr[sendIter]->totalLen + TGF_VLAN_TAG_SIZE_CNS;
                                }
                                else
                                {
                                        packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
                                }
                        }

            /* AUTODOC: verify to get 1,3 packets on port 2; 2,4 packets on port 3 */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS - 1;
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = PRV_TGF_VLANID_2_CNS;

        if(prvTgfPacketsCountTriggerArr[0][sendIter] == 0)
        {
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_EXPECTED_NO_MATCH_E;
        }

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);
        /* check TriggerCounters */

                expectTriggers = (prvTgfPacketsCountTriggerArr[0][sendIter] != 0) ?
                ((prvTgfPacketsCountTriggerArr[0][sendIter] << prvTgfBurstCount) - 1) :
                prvTgfPacketsCountTriggerArr[0][sendIter];

                UTF_VERIFY_EQUAL2_STRING_MAC(expectTriggers, numTriggers,
                                                                         "get another trigger that expected: expected - %d, recieved - %d\n",
                                                                         expectTriggers, numTriggers);
    }
}

/**
* @internal prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgEgrTagModifNonDsaTaggedEgrPortConfigRestore
(
    GT_VOID
)
{
        GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

        /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                                                 prvTgfDevNum, GT_TRUE);

        /* invalidate VLAN 2 entry */
        rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* AUTODOC: restore default PVID on port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[1],
                                 1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[1],
                                 1);
}


