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
* @file prvTgfBrgDroppingNonIpV4MacMulticastFiltering.c
*
* @brief Discard all non-Control-classified Ethernet packets
* with a MAC Multicast DA (but not the Broadcast MAC
* address) not corresponding to the IP Multicast range.
*
* @version   3
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
#include <bridge/prvTgfBrgDroppingNonIpV4MacMulticastFiltering.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/*  VLAN Id 2 */
#define PRV_TGF_VLANID_2_CNS                    2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            1

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            6

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
   {1, 0, 0, 0},
   {1, 1, 0, 0},
   {1, 1, 0, 0},
   {1, 0, 0, 0},
   {1, 1, 0, 0},
   {1, 1, 0, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
   {1, 0, 0, 0},
   {1, 1, 1, 1},
   {1, 1, 1, 1},
   {1, 0, 0, 0},
   {1, 1, 1, 1},
   {1, 1, 1, 1}
};

/* expected number of capturing packets */
static GT_U8 prvTgfTriggerExpectArr[PRV_TGF_SEND_PACKETS_NUM_CNS] = 
{
    0, 1, 1, 0, 1, 1
};

/* array of dst MACs for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS] = 
{
    {0x01, 0x00, 0x5d, 0x00, 0x00, 0x02},
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x01, 0x5a, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
};


/******************************* Test packet **********************************/
/* L2 part of IPv6 MC packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                  /* version */
    5,                                  /* headerLen */
    0,                                  /* typeOfService */
    0x2A,                               /* totalLen */
    0,                                  /* id */
    0,                                  /* flags */
    0,                                  /* offset */
    0x40,                               /* timeToLive */
    0xff,                               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    {0, 0, 0, 1},                       /* srcAddr */
    {0, 0, 0, 2}                        /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
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

/* PARTS of MC IPv4 packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArrayIpv4[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* MC IPv4 PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoIpV4 =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                                       /* totalLen */
    sizeof(prvTgfPacketPartArrayIpv4) / sizeof(TGF_PACKET_PART_STC),   /* numOfParts */
    prvTgfPacketPartArrayIpv4                                          /* partsArray */
};

/* PARTS of non IP tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] = 
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of non IP TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* non IP TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfNonIpTaggedPacketInfo = 
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/* packets to send */
static TGF_PACKET_STC* prvTgfPacketInfoArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    &prvTgfPacketInfoIpV4,
    &prvTgfPacketInfoIpV4,
    &prvTgfPacketInfoIpV4,
    &prvTgfNonIpTaggedPacketInfo,
    &prvTgfNonIpTaggedPacketInfo,
    &prvTgfPacketInfoIpV4
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigSet
(
    GT_VOID
)
{
    GT_STATUS        rc = GT_OK;
    GT_U8            tagArray[] = {1, 1, 1, 1};


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: set command to drop non-IPv4/6 Mc packets */
    rc = prvTgfBrgGenDropNonIpMcEnable(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenDropNonIpMcEnable");
}

/**
* @internal prvTgfBrgDroppingNonIpV4MacMulticastFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgDroppingNonIpV4MacMulticastFilteringTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       portIter = 0;
    GT_U8                       sendIter = 0;
    GT_U32                      packetLen = 0;
    GT_U32                      numTriggers = 0;

    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface = {0};

    /* AUTODOC: GENERATE TRAFFIC: */

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* set Dst Mac */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, 
                 prvTgfDaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter],
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: send 3 IPv4 UC\MC packets on port 0 with: */
        /* AUTODOC:   DA=[01:00:5d:00:00:02], SA=00:00:00:00:00:20, VID=2 */
        /* AUTODOC:   sIP=0.0.0.1, dIP=0.0.0.2 */
        /* AUTODOC:   DA=[01:00:5e:00:00:02], SA=00:00:00:00:00:20, VID=2 */
        /* AUTODOC:   sIP=0.0.0.1, dIP=0.0.0.2 */
        /* AUTODOC:   DA=[00:00:00:00:00:02], SA=00:00:00:00:00:20, VID=2 */
        /* AUTODOC:   sIP=0.0.0.1, dIP=0.0.0.2 */
        /* AUTODOC: send 2 UC\MC non-ip packets on port 0 with: */
        /* AUTODOC:   DA=[01:5a:00:00:00:02], SA=00:00:00:00:00:20, VID=2 */
        /* AUTODOC:   DA=[00:00:00:00:00:02], SA=00:00:00:00:00:20, VID=2 */
        /* AUTODOC: send IPv4 BC packet on port 0 with: */
        /* AUTODOC:   DA=[ff:ff:ff:ff:ff:ff], SA=00:00:00:00:00:20, VID=2 */
        /* AUTODOC:   sIP=0.0.0.1, dIP=0.0.0.2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        packetLen = prvTgfPacketInfoArr[sendIter] == &prvTgfPacketInfoIpV4 ? 
                    prvTgfPacketInfoIpV4.totalLen :
                    prvTgfNonIpTaggedPacketInfo.totalLen;

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1st MC IPv4 packet - dropped */
        /* AUTODOC:   2nd MC IPv4 packet - received on ports 1,2,3 */
        /* AUTODOC:   3nd UC IPv4 packet - received on ports 1,2,3 */
        /* AUTODOC:   4th MC non-IPv4 packet - dropped */
        /* AUTODOC:   5th UC non-IPv4 packet - received on ports 1,2,3 */
        /* AUTODOC:   6th BC IPv4 packet - received on ports 1,2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
             /* check ETH counters */
             rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                         prvTgfPortsArray[portIter],
                                         prvTgfPacketsCountRxArr[sendIter][portIter],
                                         prvTgfPacketsCountTxArr[sendIter][portIter],
                                         packetLen, prvTgfBurstCount);
             UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* set vfd for destination MAC */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount*prvTgfTriggerExpectArr[sendIter], numTriggers,
                          "get another trigger that expected: expected - %d, recieved - %d\n",
                         prvTgfBurstCount*prvTgfTriggerExpectArr[sendIter], numTriggers);
    }
}

/**
* @internal prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgDroppingNonIpV4MacMulticastFilteringConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore command to drop IPv4/6 Mc packets */
    rc = prvTgfBrgGenDropNonIpMcEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* invalidate vlan entry 2 (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);
}


