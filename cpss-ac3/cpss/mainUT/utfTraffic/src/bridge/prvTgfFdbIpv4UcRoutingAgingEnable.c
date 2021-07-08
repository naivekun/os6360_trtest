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
* @file prvTgfFdbIpv4UcRoutingAgingEnable.c
*
* @brief FDB IPv4 UC Routing + Aging configurations
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfIpGen.h>

#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>
#include <bridge/prvTgfFdbIpv4UcRoutingAgingEnable.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

static GT_HW_DEV_NUM hwDevNum = 0x10;

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS      2
/*#define PRV_TGF_NEXTHOP_2_PORT_IDX_CNS    3*/

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E


/* the Virtual Router index */
static GT_U32        prvUtfVrfId  = 1;
static GT_U32        lpmDbId      = 0;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

static GT_ETHERADDR prvTgfArpMacAddrGet;

/* default next hop packet command used for restore */
static CPSS_PACKET_CMD_ENT  prvTgfNhPacketCmdGet;

/* default refresh value for restore */
static GT_BOOL prvTgfUcIpv4RouteAgingStatusFirstCall=GT_TRUE;/* is this the first call to sav ethe value */
static GT_BOOL prvTgfUcIpv4RouteAgingStatusGet;
static PRV_TGF_MAC_ACTION_MODE_ENT prvTgfUcIpv4RouteAgingModeGet;
static GT_BOOL prvTgfUcIpv4RouteAAandTAToCpuGet;


/* flag for first default values get  - the get should be done only once */
static GT_BOOL defaultValuesFirstGet = GT_TRUE;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableConfigurationSet
(
    GT_BOOL routingByIndex
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    if(defaultValuesFirstGet==GT_TRUE)
    {
        /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
        rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum,prvTgfRouterArpIndex,&prvTgfArpMacAddrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

        rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* AUTODOC: get Packet Commands for FDB routed packets */
        rc = prvTgfBrgFdbRoutingNextHopPacketCmdGet( prvTgfDevNum,&prvTgfNhPacketCmdGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdGet");

        /* AUTODOC: set Packet Commands for FDB routed packets */
        rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum,CPSS_PACKET_CMD_ROUTE_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

        /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
        rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        defaultValuesFirstGet=GT_FALSE;
    }

    /* AUTODOC: calculate index for ipv4 uc route entry in FDB */
    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(entryKey.key.ipv4Unicast.dip, prvTgfPacketIpv4Part.dstAddr, sizeof(entryKey.key.ipv4Unicast.dip));
    /* set key virtual router */
    entryKey.key.ipv4Unicast.vrfId = prvUtfVrfId;

    /* AUTODOC: set  ipv4 uc route entry in FDB*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(&macEntry.key.key.ipv4Unicast, &entryKey.key.ipv4Unicast,sizeof(macEntry.key.key.ipv4Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirror = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = 0;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.qosProfileIndex = 0;
    macEntry.fdbRoutingInfo.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.dipAccessLevel = 0;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.mtuProfileIndex = 0;
    macEntry.fdbRoutingInfo.isTunnelStart = GT_FALSE;
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_NEXTHOPE_VLANID_CNS;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfRouterArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = hwDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];

    /* setting age bit to 1 - We expect that after first "aging trigger" value
       will be changed to 0 and after second "aging trigger" an AA_TA msg
       will be send to CPU */
    macEntry.age = GT_TRUE;

    prvTgfFdbIpv4UcRoutingRouteConfigurationSet(routingByIndex,&macEntry);
}


/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableSet function
* @endinternal
*
* @brief   Enable/Disable Aging of UC entries
*
* @param[in] agingWithRemoval         - GT_TRUE: age with removal
*                                      GT_FALSE: age without removal
* @param[in] enableFdbRoutingAging    - GT_TRUE:  Enable Aging of UC entries
*                                      GT_FALSE: Disable Aging of UC entries
* @param[in] enableFdbRoutingAAandTAToCpu - GT_TRUE  - AA and TA messages are not
*                                      forwarded to the CPU for UC route entries.
*                                      GT_FALSE - AA and TA messages are not
*                                      forwarded to the CPU for
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableSet
(
    GT_BOOL     agingWithRemoval,
    GT_BOOL     enableFdbRoutingAging,
    GT_BOOL     enableFdbRoutingAAandTAToCpu
)
{
    GT_STATUS   rc;
    GT_U32      counterVal;
    GT_U32      ipv4UcFdbEntryIndex;

    PRV_UTF_LOG3_MAC("agingWithRemoval[%d], "
                     "enableFdbRoutingAging[%d], "
                     "enableFdbRoutingAAandTAToCpu[%d]\n",
                     agingWithRemoval,
                     enableFdbRoutingAging,
                     enableFdbRoutingAAandTAToCpu);

    if(prvTgfUcIpv4RouteAgingStatusFirstCall==GT_TRUE)
    {
        /* save FDB trigger mode */
        rc = prvTgfBrgFdbMacTriggerModeGet(prvTgfDevNum, &prvTgfUcIpv4RouteAgingModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeGet");


       /* save fdb routing uc Aging status*/
        rc =  prvTgfBrgFdbRoutingUcAgingEnableGet(prvTgfDevNum,
                                                 &prvTgfUcIpv4RouteAgingStatusGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAgingEnableGet: %d", prvTgfDevNum);

        /* save fdb routing uc AAandTAToCpu status*/
        rc =  prvTgfBrgFdbRoutingUcAAandTAToCpuGet(prvTgfDevNum,
                                                 &prvTgfUcIpv4RouteAAandTAToCpuGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAAandTAToCpuGet: %d", prvTgfDevNum);
    }

    /* AUTODOC: set trigger mode: action is done via trigger from CPU */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");

    /* set Aging Enable */
    rc =  prvTgfBrgFdbRoutingUcAgingEnableSet(prvTgfDevNum,enableFdbRoutingAging);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAgingEnableSet: %d", prvTgfDevNum);

    /* set fdb routing uc AAandTAToCpu status*/
    rc =  prvTgfBrgFdbRoutingUcAAandTAToCpuSet(prvTgfDevNum,enableFdbRoutingAAandTAToCpu);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAAandTAToCpuSet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= : trigger Aging by HW =======\n");
    /* trigger Aging by HW*/
    if(agingWithRemoval==GT_TRUE)
    {
        rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E);
    }
    else
    {
        rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    }
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
    /* wait for trigger action to end */
    rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    ipv4UcFdbEntryIndex = prvTgfFdbIpv4UcFdbEntryIndexGet();
    rc =  prvTgfBrgFdbBankCounterValueGet(prvTgfDevNum, ipv4UcFdbEntryIndex%16,&counterVal);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterValueGet: %d", ipv4UcFdbEntryIndex%16);

    PRV_UTF_LOG1_MAC("after aging - prvTgfBrgFdbBankCounterValueGet counter = %d\n",counterVal);
}

/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's sendPortNum packet:
*         Success Criteria:
*         if expectTraffic== GT_TRUE --> IPv4 Uc Packet is captured on port 2
*         if expectTraffic== GT_FALSE --> IPv4 Uc Packet is not captured on port 2
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableTrafficGenerate
(
    GT_U8    sendPortNum,
    GT_U8    nextHopPortNum,
    GT_BOOL  expectTraffic
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portsCount = prvTgfPortsNum;
    GT_U32       portIter   = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          numVfd = 0;/* number of VFDs in vfdArray */
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_BOOL                         isOk;
    TGF_MAC_ADDR                    arpMacAddr;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(expectTraffic==GT_TRUE)
    {
        /* setup portInterface for capturing: nextHopPortNum is one of PRV_TGF_NEXTHOPE_VLANID_CNS members */
        portInterface.type              = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum   = nextHopPortNum;

        /* enable capture on next hop port*/
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, TGF_CAPTURE_MODE_MIRRORING_E);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }
    /*  Generating Traffic */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetPartsPtr = prvTgfPacketPartArray;

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

     /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=2.2.2.4 */
    /* AUTODOC:   dstIP=1.1.1.3 */

    /* send packet -- send from specific port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(1000);

    /* AUTODOC: check counters - packet is captured on port 2,3 */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if((prvTgfPortsArray[portIter]==sendPortNum)||
            ((expectTraffic==GT_TRUE) && (prvTgfPortsArray[portIter] == nextHopPortNum)))
        {
                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else
        {
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    if(expectTraffic==GT_TRUE)
    {
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(arpMacAddr, &prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));

        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, TGF_CAPTURE_MODE_MIRRORING_E);

        /* check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        /* number of triggers should be according to number of transmit*/
        UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                arpMacAddr[0], arpMacAddr[1], arpMacAddr[2],
                arpMacAddr[3], arpMacAddr[4], arpMacAddr[5]);
    }
}

/**
* @internal prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues function
* @endinternal
*
* @brief   Check the Valid/Skip/Age values of the inserted FDB Entry is as expected
*
* @param[in] expectedValid            - GT_TRUE: valid is 1
*                                      GT_FALSE: valid is 0
* @param[in] expectedSkip             - GT_TRUE: skip is 1
*                                      GT_FALSE: skip is 0
* @param[in] expectedAge              - GT_TRUE: age is 1
*                                      GT_FALSE: age is 0
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingCheckValidSkipAgeValues
(
    GT_BOOL expectedValid,
    GT_BOOL expectedSkip,
    GT_BOOL expectedAge
)
{
    GT_BOOL                      tmp_valid;
    GT_BOOL                      tmp_skip;
    GT_BOOL                      tmp_aged;
    GT_HW_DEV_NUM                tmp_hwDevNum;
    PRV_TGF_BRG_MAC_ENTRY_STC    tmp_macEntry;

    prvTgfFdbIpv4UcRoutingRouteConfigurationGetByIndex(&tmp_valid,
                                                       &tmp_skip,
                                                       &tmp_aged,
                                                       &tmp_hwDevNum,
                                                       &tmp_macEntry);

    PRV_UTF_LOG2_MAC("expectedValid[%d], receivedValid[%d]\n", expectedValid,tmp_valid);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedValid,tmp_valid);
    PRV_UTF_LOG2_MAC("expectedSkip[%d], receivedSkip[%d]\n", expectedSkip,tmp_skip);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedSkip, tmp_skip);
    PRV_UTF_LOG2_MAC("expectedAge[%d], receivedAge[%d]\n", expectedAge,tmp_aged);
    UTF_VERIFY_EQUAL0_PARAM_MAC(expectedAge,  tmp_aged);
}

/**
* @internal prvTgfFdbIpv4UcRoutingAgingEnableConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcRoutingAgingEnableConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc          = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore a ARP MAC  to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &prvTgfArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum,prvTgfNhPacketCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: restore FDB trigger mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(prvTgfUcIpv4RouteAgingModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");

    /* AUTODOC: restore Aging status */
    rc =  prvTgfBrgFdbRoutingUcAgingEnableSet(prvTgfDevNum,prvTgfUcIpv4RouteAgingStatusGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAgingEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore fdb routing uc AAandTAToCpu status*/
    rc =  prvTgfBrgFdbRoutingUcAAandTAToCpuSet(prvTgfDevNum,prvTgfUcIpv4RouteAAandTAToCpuGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcAAandTAToCpuSet: %d", prvTgfDevNum);

    prvTgfUcIpv4RouteAgingStatusFirstCall=GT_TRUE;

    defaultValuesFirstGet=GT_TRUE;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (0 != prvUtfVrfId)
    {
        /* Remove Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterDel(lpmDbId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", prvTgfDevNum);
    }
}




