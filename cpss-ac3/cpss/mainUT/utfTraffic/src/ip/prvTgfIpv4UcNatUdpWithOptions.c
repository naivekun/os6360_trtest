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
* @file prvTgfIpv4UcNatUdpWithOptions.c
*
* @brief NAT: ipv4 + options, unicast UDP
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
#include <common/tgfCncGen.h>


/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS       1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS 3

/* Desired nat entry index */
#define PRV_TGF_NAT_ENTRY_IDX_CNS     (5*1024 + 13)

/* Actual used nat entry index,
 *  should be less than tunnelStart table size,
 *  should be the same as ARP CNC counter index + 1024*log(indexRangeBmp)/log(2) */
static GT_U32 prvTgfNextHopNatPointer;

/* amount of sent packets */
#define PRV_TGF_PACKET_NUM_CNS 1


/* nat cnc block number */
static GT_U32 blockNumNat   = 7;

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

/* cnc block enable orig */
static GT_BOOL cncBlockEnableOrig;

/* cnc block range orig */
static GT_U32 rangesBmpArrOrig[2];

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
    0x11, /*UDP*/       /* protocol */
    0x69E4,             /* csum */
    {0xAC, 0x1F, 0x20, 0x21},   /* srcAddr */
    {0x10, 0xA6, 0xA7, 0xA8}    /* dstAddr */
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

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    0xFF11,   /* src port */
    0xEE22,   /* dst port */
    0x01BE,   /* len */
    0x0,      /* csum */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
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
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/**
* @internal prvTgfIpv4UcNatUdpWithOptionsRoutingGenerateWithCapture function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatUdpWithOptionsRoutingGenerateWithCapture
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
* @internal prvTgfIpv4UcNatUdpWithOptionsRoutingCheckCapturedByField function
* @endinternal
*
* @brief   This code is another way to do traffic check,
*         like vfd array, but shorter. In some cases it could be more convenient.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                       None
*/
static GT_VOID prvTgfIpv4UcNatUdpWithOptionsRoutingCheckCapturedByField
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
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};

    CPSS_INTERFACE_INFO_STC portInterface;

    const GT_U8  l3Checksum[] = { 0xC0, 0x8F };
    const GT_U8  l4Checksum[] = { 0x01, 0xBE };
    const GT_U32 checkSumSize = 2;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = devNum;
    portInterface.devPort.portNum  = portNum;

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
                PRV_UTF_LOG2_MAC("expected checksum 0x%X%X \n"  , l3Checksum[0], l3Checksum[1]);
                PRV_UTF_LOG2_MAC("got      checksum 0x%X%X \n\n", packetBuff[24], packetBuff[25]);
            }

            /* check ipv4 header options */
            if (0 != cpssOsMemCmp(prvTgfPacketOptionsPart.dataPtr, &(packetBuff[34]), prvTgfPacketOptionsPart.dataLength))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet ipv4 options is not as expected");
            }

            /* check l4 check sum */
            if (0 != cpssOsMemCmp(l4Checksum, &(packetBuff[62]), checkSumSize))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet l4 checksum is not as expected");
                PRV_UTF_LOG2_MAC("expected checksum 0x%X%X \n"  , l4Checksum[0], l4Checksum[1]);
                PRV_UTF_LOG2_MAC("got      checksum 0x%X%X \n\n", packetBuff[62], packetBuff[63]);
            }
        }


        {   /* check nat pass cnc counter */

            PRV_TGF_CNC_COUNTER_STC           counter;
            cpssOsMemSet(&counter, 0, sizeof(counter));

            rc = prvTgfCncCounterGet(blockNumNat, prvTgfNextHopNatPointer%1024, PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

            if(GT_OK == rc)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PACKET_NUM_CNS, counter.packetCount.l[0], "Nat counter is wrong");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, counter.packetCount.l[1], "Nat counter is wrong, l[1]");
            }
        }


    }
}

/**
* @internal prvTgfIpv4UcNatUdpWithOptionsRoutingConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatUdpWithOptionsRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: disable ingress policy on port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    /* AUTODOC: enables ingress policy */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable");

    /* Restore Route/Basic Configuration*/

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet");

    /* AUTODOC: disable UC IPv4 Routing on port 1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 1 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* AUTODOC: restore nat entry */
    rc = prvTgfIpNatEntrySet(prvTgfDevNum, prvTgfNextHopNatPointer,
                             natTypeOrig, &natEntryOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "restore prvTgfIpNatEntrySet");

    /* AUTODOC: restore nat cnc block enable value */
    rc = prvTgfCncBlockClientEnableSet(blockNumNat, PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E, cncBlockEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "restore prvTgfCncBlockClientEnableSet");

    /* AUTODOC: restore nat cnc block range */
    rc = prvTgfCncBlockClientRangesSet(blockNumNat, PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E, rangesBmpArrOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet");

    /* AUTODOC: restore a PBR leaf */
    prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntryOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "restore prvTgfLpmLeafEntryWrite");

    /* AUTODOC: restore route entry */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArrayOrig, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore enabling status of bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, pbrBypassTrigger);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
};

/**
* @internal prvTgfIpv4UcNatUdpWithOptionsRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfIpv4UcNatUdpWithOptionsRoutingLttRouteConfigurationSet
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

    prvTgfNextHopNatPointer = PRV_TGF_NAT_ENTRY_IDX_CNS %
                                (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.tunnelStart);

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_SEND_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    regularEntryPtr->isTunnelStart              = GT_TRUE;
    regularEntryPtr->isNat                      = GT_TRUE;
    regularEntryPtr->nextHopNatPointer          = prvTgfNextHopNatPointer;

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
* @internal prvTgfIpv4UcNatUdpWithOptionsRoutingCncSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatUdpWithOptionsRoutingCncSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      rangesBmpArr[2] = {0};

    /* AUTODOC: save nat cnc block enable value */
    blockNumNat %= PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.cncBlocks;
    rc = prvTgfCncBlockClientEnableGet(blockNumNat, PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E, &cncBlockEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableGet");

    /* AUTODOC: enable nat cnc block */
    rc = prvTgfCncBlockClientEnableSet(blockNumNat, PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet");

    rangesBmpArr[0] = (1 << (prvTgfNextHopNatPointer / 1024));

    /* AUTODOC: save nat cnc block range */
    rc = prvTgfCncBlockClientRangesGet(blockNumNat, PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E, rangesBmpArrOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesGet");

    /* AUTODOC: set nat cnc block range */
    rc = prvTgfCncBlockClientRangesSet(blockNumNat, PRV_TGF_CNC_CLIENT_ARP_TABLE_ACCESS_E, rangesBmpArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet");
}

/**
* @internal prvTgfIpv4UcNatUdpWithOptionsRoutingNatEntrySet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatUdpWithOptionsRoutingNatEntrySet
(
    GT_VOID
)
{
    GT_STATUS   rc;

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

    natEntry.nat44Entry.modifyTcpUdpDstPort = GT_TRUE;
    natEntry.nat44Entry.newTcpUdpDstPort    = 0x39F7;

    natEntry.nat44Entry.modifyTcpUdpSrcPort = GT_TRUE;
    natEntry.nat44Entry.newTcpUdpSrcPort    = 0x3067;

    /* AUTODOC: save orig nat entry */
    rc = prvTgfIpNatEntryGet(prvTgfDevNum, prvTgfNextHopNatPointer,
                             &natTypeOrig, &natEntryOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpNatEntryGet");

    /* AUTODOC: set nat entry */
    rc = prvTgfIpNatEntrySet(prvTgfDevNum, prvTgfNextHopNatPointer,
                             PRV_TGF_IP_NAT_TYPE_NAT44_E, &natEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpNatEntrySet");


}


/**
* @internal prvTgfIpv4UcNatUdpWithOptionsRoutingPclConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfIpv4UcNatUdpWithOptionsRoutingPclConfigurationSet
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
* @internal prvTgfIpv4UcNatUdpWithOptions function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfIpv4UcNatUdpWithOptions
(
    GT_VOID
)
{
    /* AUTODOC: Setup configuration */

    /* AUTODOC: Set pcl config */
    prvTgfIpv4UcNatUdpWithOptionsRoutingPclConfigurationSet();

    /* AUTODOC: Set ip config */
    prvTgfIpv4UcNatUdpWithOptionsRoutingLttRouteConfigurationSet();

    /* AUTODOC: Set nat config */
    prvTgfIpv4UcNatUdpWithOptionsRoutingNatEntrySet();

    /* AUTODOC: Set cnc config */
    prvTgfIpv4UcNatUdpWithOptionsRoutingCncSet();

    /* AUTODOC: Generate traffic */
    prvTgfIpv4UcNatUdpWithOptionsRoutingGenerateWithCapture();

    /* AUTODOC: verify traffic on port 3 with expected fields */
    prvTgfIpv4UcNatUdpWithOptionsRoutingCheckCapturedByField(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

    /* AUTODOC: Restore configuration */
    prvTgfIpv4UcNatUdpWithOptionsRoutingConfigurationRestore();
}


