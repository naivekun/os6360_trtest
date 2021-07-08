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
* @file prvTgfMcBridgingIpV4.c
*
* @brief Check IPM Bridging mode for Source-Specific
* Multicast (SSM) snooping
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
#include <bridge/prvTgfMcBridgingIpV4.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/*  VLAN Id 2 */
#define PRV_TGF_VLANID_2_CNS                    2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            1

/* generate traffic vidx */
#define PRV_TGF_VIDX_CNS                        1

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            3

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* MC group members */
static GT_U8 prvTgfMcMembers[] =
{
   1, 2
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_PORTS_NUM_CNS] =
{
   1, 1, 0, 0
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_PORTS_NUM_CNS] =
{
    1, 1, 1, 0
};

/* expected number of capturing packets */
static GT_U8 prvTgfTriggerExpect = 1;

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}                /* saMac */
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
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive */
    0xff,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {0xE0, 0, 0, 1},                   /* srcAddr */
    {0xE0, 0, 0, 2}                    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + \
    TGF_VLAN_TAG_SIZE_CNS + \
    sizeof(prvTgfPacketEtherTypePart) + \
    sizeof(prvTgfPacketIpv4Part) + \
    sizeof(prvTgfPayloadDataArr)

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};


/******************************************************************************/
/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static    PRV_TGF_BRG_MAC_ENTRY_STC  prvTgfMacEntry = {{0}};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfMcBridgingIpV4ConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfMcBridgingIpV4ConfigSet
(
    GT_VOID
)
{
    GT_STATUS           rc;
    GT_U32              portIter, portIdx;
    CPSS_PORTS_BMP_STC  portMembers;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
                                           prvTgfPortsArray,
                                           NULL, NULL, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portMembers);

    /* create bmp with ports for MC group */
    for (portIter = 0; portIter < sizeof(prvTgfMcMembers)/sizeof(prvTgfMcMembers[0]); portIter++)
    {
        /* use portIdx var to avoid PPC85XX compilation problem */
        portIdx = prvTgfMcMembers[portIter];
        CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[portIdx]);
    }

    /* AUTODOC: create VIDX 1 with ports 1, 2 */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_CNS, &portMembers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgMcEntryWrite");

    /* AUTODOC: enable IPv4 multicast bridging on VLAN 2 */
    rc = prvTgfBrgVlanIpmBridgingEnableSet(PRV_TGF_VLANID_2_CNS,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpmBridgingEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set IPM bridging mode to SGV mode for VLAN 2 */
    rc = prvTgfBrgVlanIpmBridgingModeSet(PRV_TGF_VLANID_2_CNS,
                                           CPSS_IP_PROTOCOL_IPV4_E, CPSS_BRG_IPM_SGV_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpmBridgingModeSet: %d", prvTgfDevNum);

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* fill mac entry by defaults */
    prvTgfMacEntry.key.entryType               = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
    prvTgfMacEntry.key.key.ipMcast.vlanId      = PRV_TGF_VLANID_2_CNS;
    prvTgfMacEntry.dstInterface.type           = CPSS_INTERFACE_VIDX_E;
    prvTgfMacEntry.dstInterface.vidx           = PRV_TGF_VIDX_CNS;
    prvTgfMacEntry.daCommand                   = CPSS_MAC_TABLE_FRWRD_E;
    prvTgfMacEntry.saCommand                   = CPSS_MAC_TABLE_FRWRD_E;
    prvTgfMacEntry.appSpecificCpuCode          = GT_TRUE;

    /* set source IP address entry */
    cpssOsMemCpy(prvTgfMacEntry.key.key.ipMcast.sip,
                 prvTgfPacketIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));

    /* set destination IP address entry */
    cpssOsMemCpy(prvTgfMacEntry.key.key.ipMcast.dip,
                 prvTgfPacketIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add FDB MC entry with sIP=224.0.0.1, dIP=224.0.0.2, VLAN 2, VIDX 1 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");
}

/**
* @internal prvTgfMcBridgingIpV4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMcBridgingIpV4TrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       portIter = 0;
    GT_U32                      packetLen = 0;
    GT_U32                      numTriggers = 0;

    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface = {0};


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

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

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv4 packet on port 0 with: */
    /* AUTODOC:   DA=01:00:5E:00:00:01, SA=00:00:00:00:00:88, VID=2 */
    /* AUTODOC:   sIP=224.0.0.1, dIP=224.0.0.2 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: verify packet received on ports 1,2 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
         packetLen = portIter != PRV_TGF_SEND_PORT_IDX_CNS ?
         prvTgfPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS :
         prvTgfPacketInfo.totalLen;

         /* check ETH counters */
         rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                     prvTgfPortsArray[portIter],
                                     prvTgfPacketsCountRxArr[portIter],
                                     prvTgfPacketsCountTxArr[portIter],
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

    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount*prvTgfTriggerExpect, numTriggers,
                      "get another trigger that expected: expected - %d, recieved - %d\n",
                     prvTgfBurstCount*prvTgfTriggerExpect, numTriggers);
}

/**
* @internal prvTgfMcBridgingIpV4ConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfMcBridgingIpV4ConfigRestore
(
    GT_VOID
)
{
    GT_STATUS           rc = GT_OK;
    CPSS_PORTS_BMP_STC  portMembers = {{0, 0}};


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* delete FDB entries */
    rc = prvTgfBrgFdbMacEntryDelete (&prvTgfMacEntry.key);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear VIDX 1 */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_CNS, &portMembers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgMcEntryWrite");

    /* AUTODOC: disable IPv4 multicast bridging */
    rc = prvTgfBrgVlanIpmBridgingEnableSet(PRV_TGF_VLANID_2_CNS,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpmBridgingEnableSet: %d", prvTgfDevNum);

    /* invalidate vlan entry 2 (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);
}


