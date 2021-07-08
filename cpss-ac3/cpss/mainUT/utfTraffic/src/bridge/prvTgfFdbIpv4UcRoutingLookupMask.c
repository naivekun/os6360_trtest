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
* @file prvTgfFdbIpv4UcRoutingLookupMask.c
*
* @brief FDB IPv4 UC Routing Lookup Mask
*
* @version   1
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
#include <trunk/prvTgfTrunk.h>

#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>
#include <bridge/prvTgfFdbIpv4UcRoutingLookupMask.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS   3

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET1_E

/* the Virtual Router index */
static GT_U32        prvUtfVrfId  = 1;
static GT_U32        lpmDbId      = 0;

/* default next hop packet command used for restore */
static CPSS_PACKET_CMD_ENT              prvTgfNhPacketCmdGet;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

static GT_ETHERADDR prvTgfArpMacAddrGet;

static GT_U32 prefixLenGet;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

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
    { 1,  1,  1,  1}    /* dstAddr */
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

/* FDB entry IPv4 destination address */
static TGF_IPV4_ADDR   dstAddr = {1, 1, 1, 0};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;


/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskRouteConfigurationSet
(
    GT_BOOL routingByIndex
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

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

    /* AUTODOC: calculate index for ipv4 uc route entry in FDB */
    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(entryKey.key.ipv4Unicast.dip, dstAddr, sizeof(entryKey.key.ipv4Unicast.dip));
    /* set key virtual router */
    entryKey.key.ipv4Unicast.vrfId = prvUtfVrfId;

    /* AUTODOC: set  ipv4 uc route entry in FDB*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
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
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

    prvTgfFdbIpv4UcRoutingRouteConfigurationSet(routingByIndex,&macEntry);

    /* AUTODOC: Save prefix length value */
    rc = prvTgfIpFdbRoutePrefixLenGet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, &prefixLenGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbRoutePrefixLenGet");
}

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB lookup prefix length
*
* @param[in] prefixLen  - FDB lookup prefix length
*
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskPrefixLengthSet
(
    GT_U32 prefixLen
)
{
    GT_STATUS rc;
    rc = prvTgfIpFdbRoutePrefixLenSet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prefixLen);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbRoutePrefixLenSet");
}

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] dstIpAddr                - packet's destination IP
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
static GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskTrafficGenerate
(
    GT_U32          sendPortNum,
    GT_U32          nextHopPortNum,
    TGF_IPV4_ADDR   dstIpAddr,
    GT_BOOL         expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    TGF_MAC_ADDR                    arpMacAddr;
    GT_TRUNK_ID currTrunkId;/* trunk Id for the current port */
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_BOOL     stormingDetected;/*indicates that the traffic to check
                   LBH that egress the trunk was originally INGRESSED from the trunk.
                   but since in the 'enhanced UT' the CPU send traffic to a port
                   due to loopback it returns to it, we need to ensure that the
                   member mentioned here should get the traffic since it is the
                   'original sender'*/
    GT_U32      vlanIter;/*vlan iterator*/
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);/*number of vlans*/
    GT_U32      numVfd = 0;/* number of VFDs in vfdArray */
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;
    GT_U32       egressPortIndex = 0xFF;
    GT_BOOL      isRoutedPacket = GT_FALSE;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        if(prvTgfPortsArray[portIter] == nextHopPortNum)
        {
            egressPortIndex = portIter;
            break;
        }
    }

    targetHwDevice = prvTgfDevNum;
    targetPort = nextHopPortNum;
    cpssOsMemCpy(&prvTgfPacketIpv4Part.dstAddr[0], &dstIpAddr[0], sizeof(TGF_IPV4_ADDR));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     *
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                            prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }


    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = sendPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK)
    {
    }
    else
    {
        senderTrunkId = 0;
    }

    /* Routing to trunk support:
       check if the NH port is trunk member.
       when trunk member --> use the trunk ID.
    */
    trunkMember.port = targetPort;
    trunkMember.hwDevice = targetHwDevice;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_TRUNK_E;
        portInterface.trunkId         = nextHopTrunkId;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            /* send 64 packets in SIP5 due to new LBH of the trunk */
            prvTgfBurstCount = 64;
        }
        else
        {
            /*since we have 2 members in the NH trunk send 8 packets*/
            prvTgfBurstCount = 8;
        }

        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 6 + 5;/* last byte of the mac SA */
        cpssOsMemSet(vfdArray[0].patternPtr,0,sizeof(TGF_MAC_ADDR));
        vfdArray[0].patternPtr[0] = 0;
        vfdArray[0].cycleCount = 1;/*single byte*/
        vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;

        numVfd = 1;
    }
    else
    {
        nextHopTrunkId = 0;

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = nextHopPortNum;
    }

    if((senderTrunkId != 0) && (nextHopTrunkId == senderTrunkId))
    {

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            /* send 64 packets in SIP5 due to new LBH of the trunk */
            prvTgfBurstCount = 64;
        }
        else
        {
            /*since we have 4 members in the NH trunk send 16 packets*/
            prvTgfBurstCount = 16;
        }

        /* make all trunk ports members of the vlans */
        for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
        {
            for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
            {
                rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                            prvTgfPortsArray[portIter], GT_FALSE);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                             prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                             prvTgfPortsArray[portIter], GT_FALSE);
            }
        }

        /* we route from a trunk to the same trunk */
        /* the 'Mirroring capture' is not good because it sets :
           force PVID = 0 for all traffic that ingress the 'Captured port' .
           but since the 'sender' is also member of the 'NH trunk' this port must
           also be 'captured' , but this will not allow the packets from the CPU
           to initiate routing at all !

           so we use 'Special PCL capture' that 'skip' the ingress vlan (before the routing)
           by this the traffic before route can ingress the device but the
           traffic after the rout will be captured to the CPU.
           */
        captureType = TGF_CAPTURE_MODE_PCL_E;

        /* state that no need to capture packets that are in the Vlan before the routing */
        rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_TRUE,PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
    }
    else
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;
    }


    /* enable capture on next hop port/trunk */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, captureType);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

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
    /* AUTODOC:   srcIP=1.1.1.1 */
    /* AUTODOC:   dstIP=1.1.1.3 for regular test, 1.1.1.3 or 3.2.2.3 for bulk test */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(1000);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* Routing trunk support:
           check if the port is trunk member.
        */
        trunkMember.port = prvTgfPortsArray[portIter];
        trunkMember.hwDevice = prvTgfDevNum;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&currTrunkId);
        if(rc == GT_OK)
        {
            /* this port is member of trunk */

            if(currTrunkId == nextHopTrunkId)
            {
                /* this port is member of the next hop trunk */

                /* we need to SUM all those ports together before we check */
                /* we do the check in trunk dedicated function for 'target trunk'*/
                /* see after this loop */
                continue;
            }
        }

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);


        if ( (prvTgfPortsArray[portIter]==sendPortNum) ||
                 ((expectTraffic==GT_TRUE)&&(portIter ==egressPortIndex)))
        {
            isRoutedPacket = GT_TRUE;
        }
        if (isRoutedPacket == GT_TRUE)
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

                isRoutedPacket = GT_FALSE;
        }
        else
        {
                 /* for other ports */
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

    if(nextHopTrunkId)
    {
        /* the traffic should egress the next hop trunk */
        /* we need to SUM all those ports together before we check */
        /* we do the check in trunk dedicated function for 'target trunk'*/
        if(nextHopTrunkId == senderTrunkId)
        {
            trunkMember.port = sendPortNum;
            trunkMember.hwDevice = prvTgfDevNum;

            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                &trunkMember,/* the port in the trunk that sent the traffic */
                &stormingDetected);/*was storming detected*/

            /* it will detect 'storming' but those are only the LBH of this port */
        }
        else
        {
            prvTgfTrunkLoadBalanceCheck(nextHopTrunkId,
                PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,
                prvTgfBurstCount,
                0,/*tolerance*/
                NULL,/*NA*/
                NULL);/*NA*/
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* disable capture on nexthope port , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                    prvTgfDevNum, captureType);
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

        /* check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        if(portInterface.type  == CPSS_INTERFACE_PORT_E)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                         portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, trunkId [%d] \n",
                                         prvTgfDevNum, nextHopTrunkId);
        }

        if(prvTgfBurstCount >= 32)
        {
            /* AUTODOC: for more then 32 transmits check that got only first 32 matches of the MAC of packet */
            UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
                "\n for more then 32 transmits check that got only first 32 matches of the MAC of packet \n");
        }
        else
        {
            /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
            /* number of triggers should be according to number of transmit*/
            UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers,
                    "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                    arpMacAddr[0], arpMacAddr[1], arpMacAddr[2],
                    arpMacAddr[3], arpMacAddr[4], arpMacAddr[5]);
        }
    }

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
        /* Check IP counters value */
        if (expectTraffic && (portIter == PRV_TGF_FDB_PORT_IDX_CNS))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(1, ipCounters.outUcRoutedPkts, "Wrong outUcRoutedPkts value");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, ipCounters.outUcRoutedPkts, "Wrong outUcRoutedPkts value");
        }
    }
    PRV_UTF_LOG0_MAC("\n");

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0/*don't care*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, GT_FALSE);

    /* just for 'cleanup' */
    captureType = TGF_CAPTURE_MODE_MIRRORING_E;

}

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskConfigurationRestore
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

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (0 != prvUtfVrfId)
    {
        /* Remove Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterDel(lpmDbId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", prvTgfDevNum);
    }

    /* AUTODOC: Save prefix length value */
    rc = prvTgfIpFdbRoutePrefixLenSet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prefixLenGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbRoutePrefixLenSet");
}

/**
* @internal prvTgfFdbIpv4UcRoutingLookupMaskTest function
* @endinternal
*
* @brief   Test FDB IPv4 Lookup Mask
*/
GT_VOID prvTgfFdbIpv4UcRoutingLookupMaskTest
(
    GT_VOID
)
{
    GT_U32 prefixLen;
    TGF_IPV4_ADDR   dstIpAddr = {1, 1, 1, 1};

    for (prefixLen = 24; prefixLen <= 32; prefixLen++)
    {
        /* Set FDB lookup prefix length */
        prvTgfFdbIpv4UcRoutingLookupMaskPrefixLengthSet(prefixLen);
        if (prefixLen == 32)
        {
            /* Generate traffic */
            prvTgfFdbIpv4UcRoutingLookupMaskTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], dstIpAddr, GT_FALSE);
        }
        else
        {
            /* Generate traffic */
            prvTgfFdbIpv4UcRoutingLookupMaskTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], dstIpAddr, GT_TRUE);
        }
    }

    dstIpAddr[3] = 128;

    for (prefixLen = 24; prefixLen <= 32; prefixLen++)
    {
        /* Set FDB lookup prefix length */
        prvTgfFdbIpv4UcRoutingLookupMaskPrefixLengthSet(prefixLen);
        if (prefixLen == 24)
        {
            /* Generate traffic */
            prvTgfFdbIpv4UcRoutingLookupMaskTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], dstIpAddr, GT_TRUE);
        }
        else
        {
            /* Generate traffic */
            prvTgfFdbIpv4UcRoutingLookupMaskTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], dstIpAddr, GT_FALSE);
        }
    }

}
