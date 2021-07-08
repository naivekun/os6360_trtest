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
* @file prvTgfMcBridgingIpV6.c
*
* @brief Check IPM Bridging mode for Any-Source Multicast
* (ASM) snooping
*
* @version   5
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
#include <bridge/prvTgfMcBridgingIpV6.h>

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

/* SIP value for test */
static     GT_U8   prvTgfSip[4] =
{
    0x00, 0x00, 0x00, 0x00
};

/* DIP value for test */
static     GT_U8   prvTgfDip[4] =
{
    0x11, 0x22, 0x33, 0x44
};

/******************************* Test packet **********************************/
/* L2 part of IPv6 MC packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartMcIpv6 =
{
    {0x33, 0x33, 0x11, 0x22, 0x33, 0x44},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* Packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartIpv6 =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* Packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketMcIpv6Part =
{
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x0111, 0x0000, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1122, 0x3344}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of MC IPv6 packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArrayMcIpv6[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartMcIpv6},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartIpv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketMcIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* MC IPv6 PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoMcIpv6 =
{
    PRV_TGF_IPV6_PACKET_LEN_CNS,                                       /* totalLen */
    sizeof(prvTgfPacketPartArrayMcIpv6) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayMcIpv6                                        /* partsArray */
};


/******************************************************************************/
/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static    PRV_TGF_BRG_MAC_ENTRY_STC  prvTgfMacEntry = {{0}};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfMcBridgingIpV6ConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfMcBridgingIpV6ConfigSet
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

    /* AUTODOC: enable IPv6 multicast bridging on VLAN 2 */
    rc = prvTgfBrgVlanIpmBridgingEnableSet(PRV_TGF_VLANID_2_CNS,
                                           CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpmBridgingEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set IPM bridging mode to GV mode for VLAN 2 */
    rc = prvTgfBrgVlanIpmBridgingModeSet(PRV_TGF_VLANID_2_CNS,
                                           CPSS_IP_PROTOCOL_IPV6_E, CPSS_BRG_IPM_GV_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpmBridgingEnableSet: %d", prvTgfDevNum);

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* fill mac entry by defaults */
    prvTgfMacEntry.key.entryType               = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
    prvTgfMacEntry.key.key.ipMcast.vlanId      = PRV_TGF_VLANID_2_CNS;
    prvTgfMacEntry.dstInterface.type           = CPSS_INTERFACE_VIDX_E;
    prvTgfMacEntry.dstInterface.vidx           = PRV_TGF_VIDX_CNS;
    prvTgfMacEntry.daCommand                   = CPSS_MAC_TABLE_FRWRD_E;
    prvTgfMacEntry.saCommand                   = CPSS_MAC_TABLE_FRWRD_E;
    prvTgfMacEntry.appSpecificCpuCode          = GT_TRUE;

    /* set source IP address entry */
    cpssOsMemCpy(prvTgfMacEntry.key.key.ipMcast.sip, prvTgfSip, sizeof(prvTgfSip));

    /* set destination IP address entry */
    cpssOsMemCpy(prvTgfMacEntry.key.key.ipMcast.dip, prvTgfDip, sizeof(prvTgfDip));

    /* AUTODOC: add FDB MC entry with sIP=0.0.0.0, dIP=0x11.0x22.0x33.0x44, VLAN 2, VIDX 1 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");
}

/**
* @internal prvTgfMcBridgingIpV6TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMcBridgingIpV6TrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       portIter = 0;
    GT_U32                      packetLen      = 0;
    GT_U32                      numTriggers    = 0;

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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoMcIpv6,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv6 packet on port 0 with: */
    /* AUTODOC:   DA=33:33:11:22:33:44, SA=00:00:00:00:00:20, VID=2 */
    /* AUTODOC:   sIP=0111:0000:1111:1111:1111:1111:1111:1111 */
    /* AUTODOC:   dIP=ff00:0000:0000:0000:0000:0000:1122:3344 */
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
         prvTgfPacketInfoMcIpv6.totalLen - TGF_VLAN_TAG_SIZE_CNS :
         prvTgfPacketInfoMcIpv6.totalLen;

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
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2PartMcIpv6.daMac, sizeof(TGF_MAC_ADDR));

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount*prvTgfTriggerExpect, numTriggers,
                      "get another trigger that expected: expected - %d, recieved - %d\n",
                     prvTgfBurstCount*prvTgfTriggerExpect, numTriggers);
}

/**
* @internal prvTgfMcBridgingIpV6ConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfMcBridgingIpV6ConfigRestore
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

    /* AUTODOC: restore IPM bridging mode */
    rc = prvTgfBrgVlanIpmBridgingModeSet(PRV_TGF_VLANID_2_CNS,
                                           CPSS_IP_PROTOCOL_IPV6_E, CPSS_BRG_IPM_SGV_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpmBridgingEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable IPv6 multicast bridging */
    rc = prvTgfBrgVlanIpmBridgingEnableSet(PRV_TGF_VLANID_2_CNS,
                                           CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpmBridgingEnableSet: %d", prvTgfDevNum);

    /* invalidate vlan entry 2 (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);
}


