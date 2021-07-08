/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId.c
*
* DESCRIPTION:
*       Verify the functionality of Tunnel Term Ipv4 and Ipv6
*         local dev source ePort/trunkId for ipv4/ipv6 key types
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
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

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS          6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT1_IDX_CNS      1

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* the TTI Rule index */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 ,3)

/* current send port index */
static GT_U32 prvTgfSendPortIdx = PRV_TGF_SEND_PORT_IDX_CNS;

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* the TTI key type used for the test */
static PRV_TGF_TTI_KEY_TYPE_ENT  keyType = PRV_TGF_TTI_KEY_UDB_IPV6_E;

/*  IPv6 Template */
static TGF_PACKET_STC packetInfo;


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypeIpv6Part = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader - no next header for IPv6 */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypeIpv4Part = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* Unicast packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */ /*0x5EA0*/
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
   ,0x00, 0x01, 0x02, 0x03, 0x04, 0x05
   ,0x00, 0x01, 0x02, 0x03, 0x04, 0x05
   ,0x00, 0x01, 0x02, 0x03, 0x04, 0x05
   ,0x00, 0x01, 0x02, 0x03, 0x04, 0x05
   ,0x00, 0x01, 0x02, 0x03, 0x04, 0x05
   ,0x00, 0x01, 0x02, 0x03, 0x04, 0x05
   ,0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypeIpv6Part},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};




/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdBridgeConfigSet function
* @endinternal
*
*
* @note creates VLAN 5 with untagged ports [0,1]
*       creates VLAN 6 with untagged ports [2,3]
*
*/
static GT_VOID tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdTtiConfigSet function
* @endinternal
*
*/
static GT_VOID tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern and Mask to match packets with Local Dev Source ePort 18*/
    ttiPattern.udbArray[0] = (GT_U8)prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS];
    ttiMask.udbArray[0]    = 0xFF;

    /* AUTODOC: set UDB byte for the key keyType: anchor Metadata, offset 22 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, keyType, 0, PRV_TGF_TTI_OFFSET_METADATA_E, 26);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);


    /* AUTODOC: set the TTI Rule Action for matched packet */
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiAction.ingressPipeBypass = GT_TRUE;

    ttiAction.tag0VlanCmd       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId        = PRV_TGF_EGR_VLANID_CNS;

    /* AUTODOC: enable the TTI lookup for keyType at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], keyType, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: enable the TTI lookup for keyType at the port 18 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS], keyType, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: add TTI rule 1 with keyType on port 0 VLAN 5 with action: redirect to egress */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, keyType, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}


/**
* @internal tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdTrafficGenerate
(
    IN GT_BOOL  expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          numOfExpectedRxPackets;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled ipv6 packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdx]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdx]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

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
           case PRV_TGF_SEND_PORT1_IDX_CNS:
                numOfExpectedRxPackets = portIter == prvTgfSendPortIdx ? prvTgfBurstCount : 0;
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
* @internal tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdConfigRestore function
* @endinternal
*
*/
static GT_VOID tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS);

    /* AUTODOC: disable TTI lookup for port 0, for keyType */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], keyType, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: disable TTI lookup for port 18, for keyType */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS], keyType, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType function
* @endinternal
*
*/
static GT_VOID tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType
(
    GT_VOID
)
{
     /* AUTODOC: build correct IPv6 Packet */
    packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo.partsArray = prvTgfPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    PRV_UTF_LOG0_MAC("======= Set vlans configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdBridgeConfigSet();


    PRV_UTF_LOG0_MAC("======= Set TTI configuration =======\n");

    /* AUTODOC: add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with */
    /* AUTODOC: TTI action: modify vid to 6, redirect to port 36 */

    /* AUTODOC: set the TTI Rule Pattern and Mask to match packets with Local Dev Source Is Trunk */
    /* AUTODOC: set UDB byte for the key keyType: anchor Metadata, offset 22 */
    /* AUTODOC: set the TTI Rule Action for matched packet */
    /* AUTODOC: enable the TTI lookup for keyType at the port 0 */
    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType */
    /* AUTODOC: add TTI rule 1 with keyType on port 0 VLAN 5 with action: redirect to egress */
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdTtiConfigSet();


    PRV_UTF_LOG0_MAC("======= Send packet from port 0, expect no TTI match =======\n");

    prvTgfSendPortIdx = PRV_TGF_SEND_PORT_IDX_CNS;

    /* AUTODOC: send packet and expect no TTI match, and no traffic on port 36  */
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdTrafficGenerate(GT_FALSE);


    PRV_UTF_LOG0_MAC("======= Send packet from port 18, expect TTI match =======\n");
    prvTgfSendPortIdx = PRV_TGF_SEND_PORT1_IDX_CNS;

    /* AUTODOC: send packet and expect TTI match, and traffic on port 36  */
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdTrafficGenerate(GT_TRUE);


    /*------------------------------------------------------------*/
    /* AUTODOC: test configuration restore */
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdConfigRestore();
}


/**
* @internal tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId function
* @endinternal
*
*/
GT_VOID tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkId
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("=========================== Ipv6 traffic ========================================\n");
    PRV_UTF_LOG0_MAC("======= Check tti key type PRV_TGF_TTI_KEY_UDB_IPV6_E ===========\n");
    keyType = PRV_TGF_TTI_KEY_UDB_IPV6_E;
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType();

    PRV_UTF_LOG0_MAC("======= Check tti key type PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E =======\n");
    keyType = PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E;
    prvTgfPacketIpv6Part.nextHeader = 6; /* TCP */
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType();

    PRV_UTF_LOG0_MAC("======= Check tti key type PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E =======\n");
    keyType = PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E;
    prvTgfPacketIpv6Part.nextHeader = 17; /* UDP */
    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType();



    PRV_UTF_LOG0_MAC("=========================== Ipv4 traffic ========================================\n");

    /* AUTODOC: change packet to ipv4 */
    prvTgfPacketPartArray[2].type    = TGF_PACKET_PART_ETHERTYPE_E;
    prvTgfPacketPartArray[2].partPtr = &prvTgfPacketEtherTypeIpv4Part;

    prvTgfPacketPartArray[3].type    = TGF_PACKET_PART_IPV4_E;
    prvTgfPacketPartArray[3].partPtr = &prvTgfPacketIpv4Part;


    PRV_UTF_LOG0_MAC("======= Check tti key type PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E ===========\n");
    keyType = PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E;

    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType();


    PRV_UTF_LOG0_MAC("======= Check tti key type PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E ===========\n");
    keyType = PRV_TGF_TTI_KEY_UDB_IPV4_TCP_E;
    prvTgfPacketIpv4Part.protocol = 6; /* TCP */

    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType();


    PRV_UTF_LOG0_MAC("======= Check tti key type PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E ===========\n");
    keyType = PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E;
    prvTgfPacketIpv4Part.protocol = 17; /* UDP */

    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType();


    PRV_UTF_LOG0_MAC("======= Check tti key type PRV_TGF_TTI_KEY_UDB_IPV4_FRAGMENT_E ===========\n");
    keyType = PRV_TGF_TTI_KEY_UDB_IPV4_FRAGMENT_E;
    prvTgfPacketIpv4Part.offset = 1; /* fragment offset */

    tgfTunnelTermIpv4AndIpv6LocalDevSourceEportTrunkIdCheckOneKeyType();

    /* Restore original packet  */
    prvTgfPacketPartArray[2].type    = TGF_PACKET_PART_ETHERTYPE_E;
    prvTgfPacketPartArray[2].partPtr = &prvTgfPacketEtherTypeIpv6Part;
    prvTgfPacketPartArray[3].type    = TGF_PACKET_PART_IPV6_E;
    prvTgfPacketPartArray[3].partPtr = &prvTgfPacketIpv6Part;
    prvTgfPacketIpv4Part.protocol = 0x04;
    prvTgfPacketIpv4Part.offset = 0;
    prvTgfPacketIpv6Part.nextHeader = 0x3b;
}


