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
* @file prvTgfTunnelTermTtiIpv4ArpUdb.c
*
* @brief Verify the functionality of Tunnel Term TTI Ipv4 ARP Metadata
*
* @version   2
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>

#include <tunnel/prvTgfTunnelTermTtiIpv4ArpUdb.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0

/* port index to receive traffic */
#define PRV_TGF_EGR_PORT_IDX_CNS   3

/* number of ports in VLans */
#define PRV_TGF_RX_SIDE_PORTS_COUNT_CNS 2
#define PRV_TGF_TX_SIDE_PORTS_COUNT_CNS 2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* send VLAN */
#define PRV_TGF_VLANID_5_CNS          5
/* receive VLAN id */
#define PRV_TGF_VLANID_6_CNS          6

/* the TTI Rule index */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 ,3)


static GT_U8 arpAnchorOffset = 1;
static GT_U8 ipAddr = 0;

/* number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

static PRV_TGF_TTI_ACTION_2_STC ttiAction;
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;

/****************************** ARP packet ************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}                 /* saMac */
};

/* ethertype part of ARP packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketArpEtherTypePart = {
                                                 TGF_ETHERTYPE_0806_ARP_TAG_CNS
};
/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacketArpPart = {
    0x01,                                   /* HW Type */
    0x0800,                                 /* Protocol (IPv4= 0x0800) */
    0x06,                                   /* HW Len = 6 */
    0x04,                                   /* Proto Len = 4 */
    0x01,                                   /* Opcode */
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},   /* nbytes: HW Address of Sender (MAC SA) */
    {0x18, 0x17, 0x16, 0x15},               /* mbytes: Protocol Address of Sender (SIP) */
    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25},   /* nbytes: HW Address of Target (MAC DA) */
    {0x14, 0x13, 0x12, 0x11}                /* mbytes: Protocol Address of Target (DIP) */
};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet ARP */
static TGF_PACKET_PART_STC prvTgfPacketArpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketArpEtherTypePart},
    {TGF_PACKET_PART_ARP_E,       &prvTgfPacketArpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Packet to send */
static TGF_PACKET_STC prvTgfPacketArpInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketArpPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketArpPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermTtiIpv4ArpUdbBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID prvTgfTunnelTermTtiIpv4ArpUdbBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

     UTF_VERIFY_EQUAL1_STRING_MAC(
         GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermTtiIpv4ArpUdbTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermTtiIpv4ArpUdbTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    /* AUTODOC: set UDB byte for the key keyType: anchor Metadata, offset 22 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     0,
                                     PRV_TGF_TTI_OFFSET_METADATA_E,
                                     arpAnchorOffset);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern and Mask to match no packets */
    ttiPattern.udbArray[0] = 0x01;
    ttiMask.udbArray[0]    = 0xFF;

    ttiAction.command = CPSS_PACKET_CMD_DROP_HARD_E;

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E on port 0 VLAN 5 with action: HARD DROP */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prvTgfTunnelTermTtiIpv4ArpUdbTtiPatternSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermTtiIpv4ArpUdbTtiPatternSet
(
    GT_U8 anchorOffset,
    GT_U8 ipAddr
)
{
    GT_STATUS rc = GT_FAIL;

    PRV_UTF_LOG0_MAC("======= Setting TTI Pattern Configuration =======\n");

    ttiPattern.udbArray[0] = ipAddr;

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: anchor Metadata, offset <anchorOffset> */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     0,
                                     PRV_TGF_TTI_OFFSET_METADATA_E,
                                     anchorOffset);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prvTgfTunnelTermTtiIpv4ArpUdbTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
*                                       None
*/
static GT_VOID prvTgfTunnelTermTtiIpv4ArpUdbTrafficGenerate
(
    IN GT_BOOL  expectTraffic
)
{
    GT_STATUS                       rc          = GT_FAIL;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          numOfExpectedRxPackets;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*  Reset all Ethernet port's counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */
    numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;

    /* AUTODOC: send 1 ethernet packet from port 0 with: VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfPacketArpInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        switch(portIter)
        {
           case PRV_TGF_SEND_PORT_IDX_CNS:
                numOfExpectedRxPackets = prvTgfBurstCount;
                break;
           case PRV_TGF_EGR_PORT_IDX_CNS:
                numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;
                break;
            default:
                numOfExpectedRxPackets = 0;
        }

        /* AUTODOC: verify counters */
        UTF_VERIFY_EQUAL3_STRING_MAC(numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0],
            "got wrong counters: port[%d], expected [%d], received[%d]\n",
             prvTgfPortsArray[portIter], numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0]);
    }
}

/**
* @internal prvTgfTunnelTermTtiIpv4ArpUdbConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
static GT_VOID prvTgfTunnelTermTtiIpv4ArpUdbConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;
    GT_U32    i = 0;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: Remove VLAN 5 members */
    for (i = 0; i < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_5_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_5_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: Remove VLAN 6 members */
    for (i = PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i < PRV_TGF_PORT_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfTunnelTermTtiIpv4ArpUdb function
* @endinternal
*
* @brief   Test of Tunnel Term TTI Dsa Metadata (main func)
*/
GT_VOID prvTgfTunnelTermTtiIpv4ArpUdb
(
    GT_VOID
)
{
    /* AUTODOC: build ARP Packet */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    prvTgfTunnelTermTtiIpv4ArpUdbBridgeConfigSet();

    /* AUTODOC: add TTI rule with command HARD DROP and Pattern with invalid udbArray */
    prvTgfTunnelTermTtiIpv4ArpUdbTtiConfigSet();

    /*------------------------------------------------------------*/
    /* AUTODOC: send packet, expect no TTI match, and traffic on port */
    PRV_UTF_LOG0_MAC("===== CASE 1: Send packet with no TTI match. Show the packet is received on port ====\n");
    prvTgfTunnelTermTtiIpv4ArpUdbTrafficGenerate(GT_TRUE);

    /* AUTODOC: Check Metadata with acnchor offset 1:4 matches packet's DIP {0x14, 0x13, 0x12, 0x11} */
    /* AUTODOC: Check Metadata with acnchor offset 5:8 matches packet's SIP {0x18, 0x17, 0x16, 0x15} */
    for (arpAnchorOffset = 1, ipAddr = 0x11; arpAnchorOffset <= 8; arpAnchorOffset++, ipAddr++)
    {
        if (arpAnchorOffset < 5)
        {
            PRV_UTF_LOG2_MAC("===== CASE %d: Checking DIP: 0x%d. Expect packet dropped =====\n", arpAnchorOffset, ipAddr);
        }
        else
        {
            PRV_UTF_LOG2_MAC("===== CASE %d. Checking SIP: 0x%d. Expect packet dropped =====\n", arpAnchorOffset, ipAddr);
        }

        /* AUTODOC: Change Anchor offset for packet's SIP\DIP */
        /* AUTODOC: send packet, expect TTI match and packet dropped */
        prvTgfTunnelTermTtiIpv4ArpUdbTtiPatternSet(arpAnchorOffset, ipAddr);
        prvTgfTunnelTermTtiIpv4ArpUdbTrafficGenerate(GT_FALSE);
    }

    /*------------------------------------------------------------*/
    /* AUTODOC: test configuration restore */
    prvTgfTunnelTermTtiIpv4ArpUdbConfigRestore();

}


