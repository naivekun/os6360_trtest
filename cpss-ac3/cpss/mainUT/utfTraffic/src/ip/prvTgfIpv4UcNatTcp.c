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
* @file prvTgfIpv4UcNatTcp.c
*
* @brief NAT: ipv4 unicast TCP
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <gtOs/gtOsTimer.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>


/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS       1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS 3

/* nat entry index */
#define PRV_TGF_NAT_ENTRY_IDX_CNS    4

/* amount of sent packets */
#define PRV_TGF_PACKET_NUM_CNS 1

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32   prvTgfRouteEntryBaseIndex = 8;

/* the leaf index in the RAM PBR section */
static GT_U32   prvTgfPbrLeafIndex = 964;

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* original enabling status of bypassing the router triggering requirements for
   PBR packets */
static GT_BOOL pbrBypassTrigger;

/* Nat entry */
static PRV_TGF_IP_NAT_ENTRY_UNT natEntry;

/* Nat entry orig */
static PRV_TGF_IP_NAT_ENTRY_UNT natEntryOrig;
static PRV_TGF_IP_NAT_TYPE_ENT  natTypeOrig;

/* Leaf entry orig */
static PRV_TGF_LPM_LEAF_ENTRY_STC leafEntryOrig;

/* Route entry orig */
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntriesArrayOrig[1];

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    0xB, /*options*/    /* headerLen */
    0,                  /* typeOfService */
    0x01EA,             /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x6,/*TCP*/         /* protocol */
    0x69EF,             /* csum */
    {0xAC, 0x1F, 0x20, 0x21},   /* srcAddr */
    {0x10, 0xA6, 0xA7, 0xA8}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    0xFF11,   /* src port */
    0xEE22,   /* dst port */
    0,        /* sequence number */
    0,        /* acknowledgment number */
    5,        /* data offset */
    0,        /* reserved */
    0,        /* flags */
    0,        /* window */
    0xE444,   /* csum */
    0         /* urgent pointer */
};

/* Options data of packet */
static GT_U8 prvTgfOptionsDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
};

/* Options part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketOptionsPart = {
    sizeof(prvTgfOptionsDataArr),   /* dataLength */
    prvTgfOptionsDataArr            /* dataPtr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06,
    0x00, 0x07, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x00, 0x0B, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x0E,
    0x00, 0x0F, 0x00, 0x10, 0x00, 0x11, 0x00, 0x12, 0x00, 0x13, 0x00, 0x14, 0x00, 0x15, 0x00, 0x16,
    0x00, 0x17, 0x00, 0x18, 0x00, 0x19, 0x00, 0x1A, 0x00, 0x1B, 0x00, 0x1C, 0x00, 0x1D, 0x00, 0x1E,
    0x00, 0x1F, 0x00, 0x20, 0x00, 0x21, 0x00, 0x22, 0x00, 0x23, 0x00, 0x24, 0x00, 0x25, 0x00, 0x26,
    0x00, 0x27, 0x00, 0x28, 0x00, 0x29, 0x00, 0x2A, 0x00, 0x2B, 0x00, 0x2C, 0x00, 0x2D, 0x00, 0x2E,
    0x00, 0x2F, 0x00, 0x30, 0x00, 0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00, 0x35, 0x00, 0x36,
    0x00, 0x37, 0x00, 0x38, 0x00, 0x39, 0x00, 0x3A, 0x00, 0x3B, 0x00, 0x3C, 0x00, 0x3D, 0x00, 0x3E,
    0x00, 0x3F, 0x00, 0x40, 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46,
    0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x4A, 0x00, 0x4B, 0x00, 0x4C, 0x00, 0x4D, 0x00, 0x4E,
    0x00, 0x4F, 0x00, 0x50, 0x00, 0x51, 0x00, 0x52, 0x00, 0x53, 0x00, 0x54, 0x00, 0x55, 0x00, 0x56,
    0x00, 0x57, 0x00, 0x58, 0x00, 0x59, 0x00, 0x5A, 0x00, 0x5B, 0x00, 0x5C, 0x00, 0x5D, 0x00, 0x5E,
    0x00, 0x5F, 0x00, 0x60, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63, 0x00, 0x64, 0x00, 0x65, 0x00, 0x66,
    0x00, 0x67, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6A, 0x00, 0x6B, 0x00, 0x6C, 0x00, 0x6D, 0x00, 0x6E,
    0x00, 0x6F, 0x00, 0x70, 0x00, 0x71, 0x00, 0x72, 0x00, 0x73, 0x00, 0x74, 0x00, 0x75, 0x00, 0x76,
    0x00, 0x77, 0x00, 0x78, 0x00, 0x79, 0x00, 0x7A, 0x00, 0x7B, 0x00, 0x7C, 0x00, 0x7D, 0x00, 0x7E,
    0x00, 0x7F, 0x00, 0x80, 0x00, 0x81, 0x00, 0x82, 0x00, 0x83, 0x00, 0x84, 0x00, 0x85, 0x00, 0x86,
    0x00, 0x87, 0x00, 0x88, 0x00, 0x89, 0x00, 0x8A, 0x00, 0x8B, 0x00, 0x8C, 0x00, 0x8D, 0x00, 0x8E,
    0x00, 0x8F, 0x00, 0x90, 0x00, 0x91, 0x00, 0x92, 0x00, 0x93, 0x00, 0x94, 0x00, 0x95, 0x00, 0x96,
    0x00, 0x97, 0x00, 0x98, 0x00, 0x99, 0x00, 0x9A, 0x00, 0x9B, 0x00, 0x9C, 0x00, 0x9D, 0x00, 0x9E,
    0x00, 0x9F, 0x00, 0xA0, 0x00, 0xA1, 0x00, 0xA2, 0x00, 0xA3, 0x00, 0xA4, 0x00, 0xA5, 0x00, 0xA6,
    0x00, 0xA7, 0x00, 0xA8, 0x00, 0xA9, 0x00, 0xAA, 0x00, 0xAB, 0x00, 0xAC, 0x00, 0xAD, 0x00, 0xAE,
    0x00, 0xAF, 0x00, 0xB0, 0x00, 0xB1, 0x00, 0xB2, 0x00, 0xB3, 0x00, 0xB4, 0x00, 0xB5, 0x00, 0xB6,
    0x00, 0xB7, 0x00, 0xB8, 0x00, 0xB9, 0x00, 0xBA, 0x00, 0xBB, 0x00, 0xBC, 0x00, 0xBD, 0x00, 0xBE,
    0x00, 0xBF, 0x00, 0xC0, 0x00, 0xC1, 0x00, 0xC2, 0x00, 0xC3, 0x00, 0xC4, 0x00, 0xC5, 0x00, 0xC6,
    0x00, 0xC7, 0x00, 0xC8, 0x00, 0xC9, 0x00, 0xCA, 0x00, 0xCB, 0x00, 0xCC, 0x00, 0xCD, 0x00, 0xCE,
    0x00, 0xCF, 0x00, 0xD0, 0x00, 0xD1, 0x00, 0xD2, 0x00, 0xD3, 0x00, 0xD4,
    0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketOptionsPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/**
* @internal prvTgfIpv4UcNatTcpRoutingGenerateWithCapture function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatTcpRoutingGenerateWithCapture
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                  partsCount  = 0;
    GT_U32                  packetSize  = 0;
    TGF_PACKET_STC          packetInfo;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset");


    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, &packetInfo,
        PRV_TGF_PACKET_NUM_CNS /*burstCount*/, 0 /*numVfd*/, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.1.1.3 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset");
}

/**
* @internal prvTgfIpv4UcNatTcpRoutingCheckCapturedByField function
* @endinternal
*
* @brief   This code is another way to do traffic check,
*         like vfd array, but shorter. In some cases it could be more convenient.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                       None
*/
static GT_VOID prvTgfIpv4UcNatTcpRoutingCheckCapturedByField
(
    IN GT_U8                devNum,
    IN GT_U32                portNum
)
{
    GT_STATUS       rc;
    GT_U8           dev      = 0;
    GT_U8           queue    = 0;
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    TGF_NET_DSA_STC rxParam;
    static GT_U8    packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    CPSS_INTERFACE_INFO_STC portInterface;

    const GT_U8  l3Checksum[] = { 0xC0, 0x9A };
    const GT_U8  l4Checksum[] = { 0xE3, 0x89 };
    const GT_U32 checkSumSize = 2;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = devNum;
    portInterface.devPort.portNum = portNum;

    buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;

    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                        TGF_PACKET_TYPE_CAPTURE_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

    if(GT_OK == rc)
    {
        /* check packet len */
        if (packetActualLength < prvTgfPacketIpv4Part.totalLen)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet len is wrong");
        }
        else  /* check packet content */
        {
            /* check mac da */
            if (0 != cpssOsMemCmp(&natEntry.nat44Entry.macDa.arEther[0], &(packetBuff[0]), 6))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet mac da is not as expected");
            }

            /* check l3 check sum */
            if (0 != cpssOsMemCmp(l3Checksum, &(packetBuff[24]), checkSumSize))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet l3 checksum is not as expected");
                PRV_UTF_LOG2_MAC("expected checksum 0x%X%X \n" , l3Checksum[0], l3Checksum[1]);
                PRV_UTF_LOG2_MAC("got      checksum 0x%X%X \n\n", packetBuff[24], packetBuff[25]);
            }

            /* check ipv4 header options */
            if (0 != cpssOsMemCmp(prvTgfPacketOptionsPart.dataPtr, &(packetBuff[34]), prvTgfPacketOptionsPart.dataLength))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet ipv4 options is not as expected");
            }

            /* check l4 check sum */
            if (0 != cpssOsMemCmp(l4Checksum, &(packetBuff[74]), checkSumSize))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet l4 checksum is not as expected");
                PRV_UTF_LOG2_MAC("expected checksum 0x%X%X \n"  , l4Checksum[0], l4Checksum[1]);
                PRV_UTF_LOG2_MAC("got      checksum 0x%X%X \n\n", packetBuff[74], packetBuff[75]);
            }

        }
    }
}

/**
* @internal prvTgfIpv4UcNatTcpRoutingConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatTcpRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: disable ingress policy on port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: enables ingress policy */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable");

    /* Restore Route/Basic Configuration*/

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet");

    /* AUTODOC: disable UC IPv4 Routing on port 1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 1 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* AUTODOC: restore nat entry */
    rc = prvTgfIpNatEntrySet(prvTgfDevNum, PRV_TGF_NAT_ENTRY_IDX_CNS,
                             natTypeOrig, &natEntryOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "restore prvTgfIpNatEntrySet");

    /* AUTODOC: restore a PBR leaf */
    prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntryOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "restore prvTgfLpmLeafEntryWrite");

    /* AUTODOC: restore route entry */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArrayOrig, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: Restore enabling status of bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, pbrBypassTrigger);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
};

/**
* @internal prvTgfIpv4UcNatTcpRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfIpv4UcNatTcpRoutingLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_LPM_LEAF_ENTRY_STC              leafEntry;

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set base configuration
     */

    /* AUTODOC: get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet");

    /* AUTODOC: set PVID 2 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable UC IPv4 Routing on port 1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: enable IPv4 UC Routing on Vlan 1 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_SEND_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    regularEntryPtr->isTunnelStart              = GT_TRUE;
    regularEntryPtr->isNat                      = GT_TRUE;
    regularEntryPtr->nextHopNatPointer          = PRV_TGF_NAT_ENTRY_IDX_CNS;

    /* AUTODOC: save route entry */
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArrayOrig, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead");

    /* AUTODOC: add UC route entry 8 with nexthop VLAN 1 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");


    /* -------------------------------------------------------------------------
     * 3. Create a LookUp Translation Table Entry
     */

    /* AUTODOC: Read a PBR leaf */
    prvTgfLpmLeafEntryRead(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntryOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfLpmLeafEntryRead");

    /* AUTODOC: write a leaf entry to the PBR area in RAM */
    cpssOsMemSet(&leafEntry, 0, sizeof(leafEntry));
    leafEntry.entryType                = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    leafEntry.index                    = prvTgfRouteEntryBaseIndex;

    /* AUTODOC: Write a PBR leaf */
    prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfLpmLeafEntryWrite");


    /* AUTODOC: Save the enabling status of bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet(prvTgfDevNum, &pbrBypassTrigger);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet");

    /* AUTODOC: Enable bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

}

/**
* @internal prvTgfIpv4UcNatTcpRoutingNatEntrySet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatTcpRoutingNatEntrySet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    cpssOsMemSet(&natEntry, 0, sizeof(natEntry));

    natEntry.nat44Entry.modifyDip = GT_FALSE;
    natEntry.nat44Entry.modifyTcpUdpDstPort = GT_FALSE;

    natEntry.nat44Entry.macDa.arEther[0] = 0x0F;
    natEntry.nat44Entry.macDa.arEther[1] = 0x1F;
    natEntry.nat44Entry.macDa.arEther[2] = 0x2F;
    natEntry.nat44Entry.macDa.arEther[3] = 0x3F;
    natEntry.nat44Entry.macDa.arEther[4] = 0x4F;
    natEntry.nat44Entry.macDa.arEther[5] = 0x5F;

    natEntry.nat44Entry.modifySip = GT_TRUE;

    natEntry.nat44Entry.newSip.arIP[0] = 0xAA;
    natEntry.nat44Entry.newSip.arIP[1] = 0xBA;
    natEntry.nat44Entry.newSip.arIP[2] = 0xCA;
    natEntry.nat44Entry.newSip.arIP[3] = 0xDA;

    natEntry.nat44Entry.modifyTcpUdpSrcPort = GT_TRUE;
    natEntry.nat44Entry.newTcpUdpSrcPort    = 0x5678;

    /* AUTODOC: save orig nat entry */
    rc = prvTgfIpNatEntryGet(prvTgfDevNum, PRV_TGF_NAT_ENTRY_IDX_CNS,
                             &natTypeOrig, &natEntryOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpNatEntryGet");

    /* AUTODOC: set nat entry */
    rc = prvTgfIpNatEntrySet(prvTgfDevNum, PRV_TGF_NAT_ENTRY_IDX_CNS,
                             PRV_TGF_IP_NAT_TYPE_NAT44_E, &natEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpNatEntrySet");
}


/**
* @internal prvTgfIpv4UcNatTcpRoutingPclConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatTcpRoutingPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS      rc = GT_OK;

    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(PRV_TGF_PCL_RULE_FORMAT_UNT));
    /* define mask, pattern and action */
    cpssOsMemSet(&pattern, 0, sizeof(PRV_TGF_PCL_RULE_FORMAT_UNT));

    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(PRV_TGF_PCL_ACTION_STC));
    action.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassIngressPipe             = GT_FALSE;
    action.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
    action.redirect.data.routerLttIndex  = prvTgfPbrLeafIndex;

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
    /* AUTODOC:   redirect to routerLtt entry 8 */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        prvTgfPclRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC( GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclRuleIndex);
};

/**
* @internal prvTgfIpv4UcNatTcpRouting function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfIpv4UcNatTcpRouting
(
    GT_VOID
)
{
    /* AUTODOC: Setup configuration */

    /* AUTODOC: Set pcl config */
    prvTgfIpv4UcNatTcpRoutingPclConfigurationSet();

    /* AUTODOC: Set ip config */
    prvTgfIpv4UcNatTcpRoutingLttRouteConfigurationSet();

    /* AUTODOC: Set nat config */
    prvTgfIpv4UcNatTcpRoutingNatEntrySet();

    /* AUTODOC: Generate traffic */
    prvTgfIpv4UcNatTcpRoutingGenerateWithCapture();

    /* AUTODOC: verify traffic on port 3 with expected fields */
    prvTgfIpv4UcNatTcpRoutingCheckCapturedByField(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

    /* AUTODOC: Restore configuration */
    prvTgfIpv4UcNatTcpRoutingConfigurationRestore();
}


