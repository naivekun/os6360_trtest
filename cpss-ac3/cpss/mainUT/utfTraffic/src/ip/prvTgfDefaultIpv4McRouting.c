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
* @file prvTgfDefaultIpv4McRouting.c
*
* @brief Default IPV4 MC Routing
*
* @version   29
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfDefaultIpv4McRouting.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* set appropriate bit for triggers checking */
#define PRV_TGF_TRIGGERS_BIT_SET_MAC(whichPacket, vfdArrayIndex, numVfd) \
        (1 << (((whichPacket) - 1) * (numVfd) + (vfdArrayIndex)))

/* send VLAN */
#define PRV_TGF_SEND_VLANID_CNS           80

/* multicast subscriber VLAN */
#define PRV_TGF_NEXTHOP_VLANID_CNS        55

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         2

/* port index to receive traffic from */
#define PRV_TGF_RCV_PORT_IDX_CNS          1

/* default port index to receive traffic from */
#define PRV_TGF_DEF_RCV_PORT_IDX_CNS      3

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS      2

/* default vidx number */
#define PRV_TGF_DEF_VIDX_CNS              0

#define SECOND_HEMISPHERE_CNS   64

/* is this a call from Dual HW Device test */
static GT_BOOL       isDualHwDevTest = GT_FALSE;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* MLL pair pointer index */
static GT_U32        prvTgfMllPointerIndex     = 5;

/* Default Route entry index for MC Route entry Table */
static GT_U32        prvTgfDefRouteEntryIndex  = 1;

/* the Route entry index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 10;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* expected number of packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountRxTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 1, 1, 1/* 1 flood in ingress vlan */},
    {0, 0, 1, 2/* 1 routed , 1 flood in ingress vlan */}
};

static GT_U8 prvTgfPacketsCountRxTxForDualHwDevArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 2, 1, 1/* 1 routed to port 1 , 1 flood in ingress vlan to port 1 and 3 */},
    {0, 1, 1, 2/* 1 routed to port 3 , 1 flood in ingress vlan to port 1 and 3 */}
};

/* expected number of packets on MLL based bridging */
static GT_U8 prvTgfPacketsCountMllBridgeRxTxArr[PRV_TGF_PORTS_NUM_CNS] =
{
    0, 0, 1, 1/* 1 bridged to MLL destination interface */
};

/* Source IP addresses for packets */
static TGF_IPV4_ADDR  prvTgfSrcAddrArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    {1, 1, 1, 1},
    {2, 2, 2, 2}
};

/* port indexes for MC subscribers */
static GT_U8 prvTgfMcPortsIdxArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_RCV_PORT_IDX_CNS, PRV_TGF_DEF_RCV_PORT_IDX_CNS
};

/* ports numbers for multi hemisphere devices */
static GT_U32    testPortsForHwDevArr[PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS] =
{
    0, 34, 10+SECOND_HEMISPHERE_CNS, 58+SECOND_HEMISPHERE_CNS

};

/* config restore ports array */
static GT_U32   prvTgfDefPortsArr[PRV_TGF_MAX_PORTS_NUM_CNS] = {0};

/* config restore number of ports array */
static GT_U8   prvTgfDefPortsNum = 0;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1, 1, 1},                    /* srcAddr = ipSrc */
    { 224,1, 1, 1}                     /* dstAddr = ipGrp */
};

/* DATA of packet len = 22 (0x16) */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U16              vid;
    CPSS_PORTS_BMP_STC  defPortMembers;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfDefaultIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfDefaultIpv4McRoutingBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS           rc          = GT_OK;
    CPSS_PORTS_BMP_STC  portMembers = {{0, 0}};

    /* Note:
        Due to compiler bug the variable portIter should be defined as volatile and a
        temporary variable, portNum and mcPortsIdx is needed instead of making assignments
        directly to prvTgfPortsArray[prvTgfMcPortsIdxArr[portIter]]
    */

    volatile GT_U32 portIter;/* volatile due to compiler bug */
    GT_U8    mcPortsIdx;     /* index of portNum in prvTgfPortsArray */
    GT_U32    portNum;        /* value of port taken from prvTgfPortsArray[mcPortsIdx] */


    /* AUTODOC: SETUP CONFIGURATION: */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portMembers);

    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
       (isDualHwDevTest == GT_TRUE))
    {
        /* AUTODOC: create VLAN 80 with untagged ports 1,2,3 */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray + 1, NULL, NULL, 3);
    }
    else
    {
        /* AUTODOC: create VLAN 80 with untagged ports 2,3 */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                               prvTgfPortsArray + 2, NULL, NULL, 2);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 55 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
                                           prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID 80 for port 2 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* save default port members for MC group */
    rc = prvTgfBrgMcEntryRead(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                              &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryRead: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* create bmp with ports for MC group */
    for (portIter = 0; portIter < PRV_TGF_SEND_PACKETS_NUM_CNS; portIter++)
    {
        mcPortsIdx = prvTgfMcPortsIdxArr[portIter];
        portNum = prvTgfPortsArray[mcPortsIdx];
        CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, portNum);
    }

    /* write MC Group entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS, &portMembers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* create a static macEntry in SEND VLAN */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_SEND_VLANID_CNS,
                                          PRV_TGF_DEF_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

/**
* @internal prvTgfDefaultIpv4McRoutingDefaultLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfDefaultIpv4McRoutingDefaultLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC             *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
	
    /* AUTODOC: enable IPv4 Multicast Routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Multicast Routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
                                   CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: create MC Link List with index 0, port 3, VLAN 80 */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_SEND_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;
    mllPairEntry.nextPointer                     = 0;

    rc = prvTgfIpMllPairWrite(0,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
	{    
       rc = cpssDxChIpRouterVlanMacSaLsbSet(prvTgfDevNum,PRV_TGF_SEND_VLANID_CNS,PRV_TGF_SEND_VLANID_CNS);  
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterVlanMacSaLsbSet: %d", prvTgfDevNum);  
	}
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr = 0;

    /* AUTODOC: add default MC route entry with index 1 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfDefRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");
}

/**
* @internal prvTgfDefaultIpv4McRoutingDefaultTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic to defaults
*/
GT_VOID prvTgfDefaultIpv4McRoutingDefaultTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc        = GT_OK;
    GT_U32                      portIter  = 0;
    GT_U32                      packetLen = 0;
    PRV_TGF_IP_COUNTER_SET_STC  ipCounters;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset IP couters and set ROUTE_ENTRY mode */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=80, srcIP=1.1.1.1, dstIP=224.1.1.1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check ETH counter of ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* calculate packet length */
        packetLen = prvTgfPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS);

        if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
          (isDualHwDevTest == GT_TRUE))
        {
            /* AUTODOC: in case of Lion2 the test tgfDefaultIpv4McRoutingForDualHwDev
            need to verify 3 packets -
            1 routed to default port 3,
            1 flood in ingress VLAN 80 to port 3
            1 flood in ingress VLAN 80 to port 1 */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        (GT_U8) (portIter == PRV_TGF_SEND_PORT_IDX_CNS),
                                        prvTgfPacketsCountRxTxForDualHwDevArr[1][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        }
        else
        {   /* AUTODOC: in case of other devices or Lion2 the test tgfDefaultIpv4McRouting
            need to verify 2 packets -
            1 routed to default port 3,
            1 flood in ingress VLAN 80 */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        (GT_U8) (portIter == PRV_TGF_SEND_PORT_IDX_CNS),
                                        prvTgfPacketsCountRxTxArr[1][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }
    }

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfDefaultIpv4McRoutingMllBridgeEnableTraffic function
* @endinternal
*
* @brief   Generate traffic with MLL based bridging
*/
GT_VOID prvTgfDefaultIpv4McRoutingMllBridgeEnableTraffic
(
    GT_VOID
)
{
    GT_STATUS                   rc        = GT_OK;
    GT_U32                      portIter  = 0;
    GT_U32                      packetLen = 0;

    /* AUTODOC: Enable MLL based bridging */
    rc = prvTgfIpMllBridgeEnable(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllBridgeEnable - enable");

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=80, srcIP=1.1.1.1, dstIP=224.1.1.1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check ETH counter of ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* calculate packet length */
        packetLen = prvTgfPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS);

        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    (GT_U8) (portIter == PRV_TGF_SEND_PORT_IDX_CNS),
                                    prvTgfPacketsCountMllBridgeRxTxArr[portIter],
                                    packetLen, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* AUTODOC: Disable MLL based bridging */
    rc = prvTgfIpMllBridgeEnable(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllBridgeEnable - restore");

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfDefaultIpv4McRoutingAdditionalLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_VOID prvTgfDefaultIpv4McRoutingAdditionalLttRouteConfigurationSet
(
    GT_U32 testNumber
)
{
    GT_STATUS                       rc = GT_OK;
    GT_IPADDR                       ipGrp, ipSrc;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC              *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;
    GT_U32                          ipGroupPrefix = 0;
    GT_U32                          ipSrcPrefix = 0;
    GT_BOOL                         override = GT_FALSE;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    cpssOsMemSet(ipGrp.arIP, 0, sizeof(ipGrp.arIP));
    cpssOsMemSet(ipSrc.arIP, 0, sizeof(ipSrc.arIP));

    /* AUTODOC: create MC Link List with index 5, port 1, VLAN 55 */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));


    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_NEXTHOP_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;
    mllPairEntry.nextPointer                     = 0;

    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {   
        rc = cpssDxChIpRouterVlanMacSaLsbSet(prvTgfDevNum,PRV_TGF_NEXTHOP_VLANID_CNS,PRV_TGF_NEXTHOP_VLANID_CNS);  
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpRouterVlanMacSaLsbSet: %d", prvTgfDevNum);   
    }


    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;

    /* AUTODOC: add default MC route entry with index 10 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the LPM */
    if (testNumber == 0)
    {
        ipGroupPrefix = 32;
        ipSrcPrefix = 32;
        cpssOsMemCpy(ipGrp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipGrp.arIP));
        cpssOsMemCpy(ipSrc.arIP, prvTgfSrcAddrArr[0], sizeof(ipSrc.arIP));
        override = GT_FALSE;
    }
    if (testNumber == 1)
    {
        ipGroupPrefix = 4;
        ipSrcPrefix = 0;
        ipGrp.arIP[0] = 224;
        override = GT_TRUE;
    }

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.1.1.1/32, ipSrc=1.1.1.1/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, ipGroupPrefix, ipSrc, ipSrcPrefix,
                                   &mcRouteLttEntry, override, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfDefaultIpv4McRoutingAdditionalRouteConfigurationSet function
* @endinternal
*
* @brief   Set additional Route Configuration
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfDefaultIpv4McRoutingAdditionalRouteConfigurationSet
(
    GT_U32 testNumber
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet");

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            PRV_UTF_LOG0_MAC("==== MC IS NOT SUPPORTED IN PBR MODE ====\n");
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            prvTgfDefaultIpv4McRoutingAdditionalLttRouteConfigurationSet(testNumber);
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:
        default:
            PRV_UTF_LOG0_MAC("==== ROUTING MODE IS NOT DEFINED ====\n");
    }
}

/**
* @internal prvTgfDefaultIpv4McRoutingAdditionalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] testNumber               - number of test
*                                       None
*/
GT_VOID prvTgfDefaultIpv4McRoutingAdditionalTrafficGenerate
(
    GT_U32 testNumber
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    GT_U32                          sendIter    = 0;
    GT_U32                          numTriggers = 0;
    GT_U32                          numTriggers2;
    TGF_VFD_INFO_STC                vfdArray[3];
    GT_U32                          numVfd;
    GT_U32                          expTriggers;
    GT_U32                          expPackets;
    CPSS_INTERFACE_INFO_STC         portInterface;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          mllOutMCPkts;
    GT_U32                          mllCntSet_0 = 0;
    GT_U32                          packetLen   = 0;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;
    GT_U32                          sendPacketsNumber = PRV_TGF_SEND_PACKETS_NUM_CNS;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));
    if (testNumber == 1)
    {
        sendPacketsNumber = 1;
    }
    /* AUTODOC: GENERATE TRAFFIC: */
    for (sendIter = 0; sendIter < sendPacketsNumber; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture on a MC subscriber port */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[prvTgfMcPortsIdxArr[sendIter]];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        if ((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
        {
            /* reset MLL counters for MC subscriber */
            rc = prvTgfIpMllCntSet(prvTgfDevNum, mllCntSet_0 + sendIter, 0);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntSet");
        }

        /* set MLL counters mode for MC subscriber */
        cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
        cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
        cntIntCfg.devNum           = prvTgfDevNum;
        cntIntCfg.portTrunkCntMode = PRV_TGF_IP_PORT_CNT_MODE_E;

        /* need to remove casting and to fix code after lion2 development is done
        PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC shoud be changed to fit same
        interface declaration as CPSS_INTERFACE_INFO_STC*/
        CPSS_TBD_BOOKMARK_EARCH

        cntIntCfg.portTrunk.port   = (GT_U8)portInterface.devPort.portNum;

        rc = prvTgfIpSetMllCntInterface(mllCntSet_0 + sendIter, &cntIntCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

        /* reset IP couters and set ROUTE_ENTRY mode */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        /* fill source IP address for packet */
        cpssOsMemCpy(prvTgfPacketIpv4Part.srcAddr, prvTgfSrcAddrArr[sendIter],
                     sizeof(prvTgfPacketIpv4Part.srcAddr));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: send 2 IPv4 packets from port 2 with: */
        /* AUTODOC:   DA=01:00:5E:01:01:01, SA=00:00:00:00:00:33 */
        /* AUTODOC:   VID=80, srcIP=[1.1.1.1,2.2.2.2] dstIP=224.1.1.1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        if (GT_TRUE == prvUtfIsGmCompilation())
            cpssOsTimerWkAfter(1000);

        /* disable capture on a MC subscriber port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* check ETH counter of ports for MC subscriber */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* calculate packet length */
            packetLen = prvTgfPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS * (portIter != PRV_TGF_SEND_PORT_IDX_CNS);

            if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
               (isDualHwDevTest == GT_TRUE))
            {
                /* AUTODOC: in case of Lion2 the test tgfDefaultIpv4McRoutingForDualHwDev
                            verify traffic:                                         */
                /* AUTODOC:   first packet - received on ports 1 due to routing,
                                             1 and 3 due to flood in vlan 80 */
                /* AUTODOC:   2 packet - 1 routed to default port 3,
                                         1 flood in ingress VLAN 80 port 1
                                         1 flood in ingress VLAN 80 port 3 */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            (GT_U8)
                                            ((portIter == PRV_TGF_SEND_PORT_IDX_CNS) ? 1 : /* rx due to sender*/
                                            (portIter == prvTgfMcPortsIdxArr[sendIter]) ? prvTgfPacketsCountRxTxForDualHwDevArr[sendIter][portIter] /* rx due to capture == Tx Num */:
                                            0),
                                            prvTgfPacketsCountRxTxForDualHwDevArr[sendIter][portIter],
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
            else
            {
                /* AUTODOC: in case of other devices or Lion2 the test tgfDefaultIpv4McRouting
                    verify traffic:                                                             */
                /* AUTODOC:   1 packet - received on ports 1,3 */
                /* AUTODOC:   2 packet - 1 routed to default port 3, 1 flood in ingress VLAN 80 */
                rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                            prvTgfPortsArray[portIter],
                                            (GT_U8)
                                            ((portIter == PRV_TGF_SEND_PORT_IDX_CNS) ? 1 : /* rx due to sender*/
                                            (portIter == prvTgfMcPortsIdxArr[sendIter]) ? prvTgfPacketsCountRxTxArr[sendIter][portIter] /* rx due to capture == Tx Num */:
                                            0),
                                            prvTgfPacketsCountRxTxArr[sendIter][portIter],
                                            packetLen, prvTgfBurstCount);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
            }
        }

        /* check captured packets */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* clear array */
        cpssOsMemSet(&vfdArray[0], 0, sizeof(vfdArray));

        /* set pattern of MAC DA in both captured packets */

        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
        vfdArray[0].mode       = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

        /* set pattern of the last byte in MAC SA in first captured packet */
        vfdArray[1].mode          = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
        vfdArray[1].offset        = 11;
        vfdArray[1].cycleCount    = 1;
        vfdArray[1].patternPtr[0] = 0x50;

        /* set pattern of the last byte in MAC SA in second captured packet */
        vfdArray[2].mode          = TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E;
        vfdArray[2].offset        = 11;
        vfdArray[2].cycleCount    = 1;
        vfdArray[2].patternPtr[0] = 0x33;

        if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
           (isDualHwDevTest == GT_TRUE))
        {
            if (sendIter==0)
            {
                vfdArray[1].patternPtr[0] = 0x37;
            }
            else
            {
                vfdArray[1].patternPtr[0] = 0x50;
            }
        }

        /* number of expected packets may be 1 due to route
           or also another one due to flood in vlan */
        expPackets = prvTgfPacketsCountRxTxArr[sendIter][prvTgfMcPortsIdxArr[sendIter]];

        /* in the first iteration the test expects only one flooded packet:
         *   check only MAC DA in vfdArray[0] for the only one packet;
         * in the second iteration the test expects two packets - flooded and routed:
         *   check MAC DA in vfdArray[0] for both packets;
         *   check MAC SA for the first captured packet in vfdArray[1];
         *   check MAC SA for the second captured packet in vfdArray[2];
         */
        numVfd = (expPackets == 2) ? 3 : 1;

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, numVfd, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = 0x%02X, rc = 0x%02X\n\n", numTriggers, rc);

        /* in some devices the flooded packet arrive before the routed packet,
           so need to swap the expected values */
        vfdArray[1].patternPtr[0] = 0x33;
        vfdArray[2].patternPtr[0] = 0x50;
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, numVfd, vfdArray, &numTriggers2);
        PRV_UTF_LOG2_MAC("    numTriggers2 = 0x%02X, rc = 0x%02X\n\n", numTriggers2, rc);
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* check that MAC DA in packet #1 matches with vfdArray[0] */
        expTriggers = PRV_TGF_TRIGGERS_BIT_SET_MAC(1, 0, numVfd);
        UTF_VERIFY_EQUAL6_STRING_MAC(expTriggers, numTriggers & expTriggers,
            "\n   MAC DA of captured packet #1 must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfPacketL2Part.daMac[0], prvTgfPacketL2Part.daMac[1], prvTgfPacketL2Part.daMac[2],
            prvTgfPacketL2Part.daMac[3], prvTgfPacketL2Part.daMac[4], prvTgfPacketL2Part.daMac[5]);

        if (expPackets == 2)
        {
            /* check that MAC DA in packet #2 matches with vfdArray[0] */
            expTriggers = PRV_TGF_TRIGGERS_BIT_SET_MAC(2, 0, numVfd);
            UTF_VERIFY_EQUAL6_STRING_MAC(expTriggers, numTriggers & expTriggers,
                "\n   MAC DA of captured packet #2 must be: %02X:%02X:%02X:%02X:%02X:%02X",
                prvTgfPacketL2Part.daMac[0], prvTgfPacketL2Part.daMac[1], prvTgfPacketL2Part.daMac[2],
                prvTgfPacketL2Part.daMac[3], prvTgfPacketL2Part.daMac[4], prvTgfPacketL2Part.daMac[5]);

            /* check that MAC SA in packet #1 matches with vfdArray[1] */
            expTriggers = PRV_TGF_TRIGGERS_BIT_SET_MAC(1, 1, numVfd);
            if ((expTriggers != (numTriggers & expTriggers)) &&
                (expTriggers != (numTriggers2 & expTriggers)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(expTriggers, numTriggers & expTriggers,
                    "\n   MAC SA of routed captured packet #1 must be: ...:%02X",
                    vfdArray[1].patternPtr[0]);
            }

            /* check that MAC SA in packet #2 matches with vfdArray[2] */
            expTriggers = PRV_TGF_TRIGGERS_BIT_SET_MAC(2, 2, numVfd);
            if ((expTriggers != (numTriggers & expTriggers)) &&
                (expTriggers != (numTriggers2 & expTriggers)))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(expTriggers, numTriggers & expTriggers,
                    "\n   MAC SA of flooded captured packet #2 must be: ...:%02X",
                    vfdArray[2].patternPtr[0]);
            }
        }

        /* AUTODOC: get and print ip counters values */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
            prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
        }
        PRV_UTF_LOG0_MAC("\n");

        /* get MLL counters for MC subscriber */
        rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0 + sendIter, &mllOutMCPkts);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
        PRV_UTF_LOG2_MAC("MC subscriber %d: mllOutMCPkts = %d\n", sendIter, mllOutMCPkts);
    }
}

/**
* @internal prvTgfDefaultIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfDefaultIpv4McRoutingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc       = GT_OK;
    GT_U32    mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore MC Route entry in index 1*/
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET0_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr = 0;

    /* AUTODOC: restore default MC route entry with index 1 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfDefRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

    /* flash all MC Ipv4 prefix */
    rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush");

    /* AUTODOC: disable multicast IPv4 routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable multicast IPv4 routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* 1. Restore Base Configuration */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* restore MC Group entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                               &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

    /* invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_SEND_VLANID_CNS);

    /* invalidate nexthop VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_NEXTHOP_VLANID_CNS);

    /* set default MLL counters mode */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));

    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

}

/**
* @internal prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationInit function
* @endinternal
*
* @brief   Set configuration for multi hemisphere devices:
*         - set specific ports numbers
*/
GT_VOID prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationInit
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;


    /* save ports array */
    cpssOsMemCpy(prvTgfDefPortsArr, prvTgfPortsArray,
                 sizeof(prvTgfPortsArray[0]) * PRV_TGF_MAX_PORTS_NUM_CNS);

    /* save number of ports array */
    prvTgfDefPortsNum = prvTgfPortsNum;

    /* AUTODOC: config ports - tx port in 1 hemisphere and rest in other */
    rc = prvTgfDefPortsArraySet(testPortsForHwDevArr, PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArraySet: %d", prvTgfDevNum);

    isDualHwDevTest = GT_TRUE;
}

/**
* @internal prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationRestore function
* @endinternal
*
* @brief   Restore config for multi hemisphere devices
*/
GT_VOID prvTgfDefaultIpv4McRoutingForDualHwDevConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: restore default ports configuration */
    rc = prvTgfDefPortsArraySet(prvTgfDefPortsArr, prvTgfDefPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArraySet: %d", prvTgfDevNum);

    /* restore MC Route entry in index 1*/
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET0_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr = 0;

    /* AUTODOC: restore default MC route entry with index 1 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfDefRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");
}

