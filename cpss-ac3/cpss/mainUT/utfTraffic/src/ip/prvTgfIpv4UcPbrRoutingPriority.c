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
* @file prvTgfBasicIpv4UcRouting.c
*
* @brief Basic IPV4 UC Routing
*
* @version   60
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
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpv4UcRoutingPriotiry.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>

/* indication for Galtis wrappers call the WA 'B' APIs */
extern GT_U32    appDemo_trunk_phy1690_WA_B;
extern GT_STATUS trunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
);
extern void prvTgfCaptureForceTtiDisableModeSet(
    IN GT_BOOL                  forceTtiLookupUnchanged
);
extern GT_STATUS prvTgfDxChPclCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
);


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           1

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLAN_ID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS 3

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32   prvTgfRouteEntryBaseIndex = 8;

/* the leaf index in the RAM PBR section */
static GT_U32   prvTgfPbrLeafIndex = 964;

static PRV_TGF_PCL_RULE_FORMAT_UNT mask;
static PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
static PRV_TGF_PCL_ACTION_STC      action;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U16         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOP_VLAN_ID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

static GT_U32        prvTgfFdbRoutingArpIndex  = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfPbrArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x33};
static TGF_MAC_ADDR  prvTgfFdbArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x44};

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

TGF_IPV4_ADDR matchDstAddr = { 1,  1,  1,  3};
TGF_IPV4_ADDR noMatchDstAddr = {1, 1, 1, 4};

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

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_MAC_HASH_FUNC_MODE_ENT              hashModeOrig;
    GT_BOOL                                     fdbRoutingPortEnableOrig;
    GT_BOOL                                     ipv4UcPortRoutingEnableOrig;
    GT_ETHERADDR                                pbrArpMacAddrOrig;
    GT_ETHERADDR                                fdbArpMacAddrOrig;
    CPSS_PACKET_CMD_ENT                         nhPacketCmdOrig;
    GT_BOOL                                     pbrBypassTriggerOrig;
    GT_BOOL                                     fdbUnicastRouteForPbrOrig;
} prvTgfRestoreCfg;
/* parameters that is needed to be restored */


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId    - virtual router ID
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityBaseConfigurationSet
(
    GT_U32  prvUtfVrfId
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create virtual router */
    if (0 != prvUtfVrfId)
    {
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;

        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            if ( (defIpv4UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E)||
                 (defIpv4UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E) )
            {
                defIpv4UcRouteEntryInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
            }
            if ( (defIpv6UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E)||
                 (defIpv6UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E) )
            {
                defIpv6UcRouteEntryInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
            }

        }

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd: %d", prvTgfDevNum);
    }

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLAN_ID_CNS,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);


    /* AUTODOC: set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_SEND_VLANID_CNS;


    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table entry index 1: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");
}

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] matchFdb              - match FDB route entry
* @param[in] expectFdbRouting      - expect FDB routing if value is GT_TRUE
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityTrafficGenerate
(
    GT_BOOL   matchFdb,
    GT_BOOL   expectFdbRouting
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
    GT_U32      egressPortIndex = 0xFF;
    GT_BOOL     isRoutedPacket = GT_FALSE;
    GT_BOOL     is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE = GT_FALSE;
    GT_U32      sendPortNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32      nextHopPortNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];


    if(matchFdb)
    {
        cpssOsMemCpy(&prvTgfPacketIpv4Part.dstAddr, &matchDstAddr, sizeof(TGF_IPV4_ADDR));
    }
    else
    {
        cpssOsMemCpy(&prvTgfPacketIpv4Part.dstAddr, &noMatchDstAddr, sizeof(TGF_IPV4_ADDR));
    }

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
            /* reset IP couters and set ROUTE_ENTRY mode */
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
           traffic after the route will be captured to the CPU.
           */
        captureType = TGF_CAPTURE_MODE_PCL_E;

        /* state that no need to capture packets that are in the Vlan before the routing */
        rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_TRUE,PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

        if(appDemo_trunk_phy1690_WA_B)
        {
            GT_PORT_NUM eportOfTrunk;

            rc = trunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                         prvTgfDevNum, senderTrunkId);
            /* we must not kill the TTI lookups on the ingress port !!! */
            /* the WA is based on it */
            prvTgfCaptureForceTtiDisableModeSet(GT_TRUE);
            is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE = GT_TRUE;
            /* set PCL for the TRAP on the srcEPort */
            rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                         prvTgfDevNum, eportOfTrunk);
        }
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
    rc = prvTgfSetTxSetup2Eth(1, 1);
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


        /* AUTODOC: verify routed packet on port 3 */
        if ( (prvTgfPortsArray[portIter]==sendPortNum) || (portIter ==egressPortIndex) )
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
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

    /* get trigger counters where packet has MAC DA as prvTgfPbrArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

    if (expectFdbRouting)
        cpssOsMemCpy(arpMacAddr, &prvTgfFdbArpMac, sizeof(TGF_MAC_ADDR));
    else
        cpssOsMemCpy(arpMacAddr, &prvTgfPbrArpMac, sizeof(TGF_MAC_ADDR));

    cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));

    /* disable capture on nexthope port , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                    prvTgfDevNum, captureType);

    if(is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE)
    {
        CPSS_INTERFACE_INFO_STC eport_portInterface = portInterface;
        /* restore settings */
        prvTgfCaptureForceTtiDisableModeSet(GT_FALSE);
        if(appDemo_trunk_phy1690_WA_B)
        {
            GT_PORT_NUM eportOfTrunk;

            rc = trunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                            prvTgfDevNum, senderTrunkId);
            /* unset PCL for the TRAP on the srcEPort */
            rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                            prvTgfDevNum, eportOfTrunk);

            eport_portInterface.type = CPSS_INTERFACE_PORT_E;
            eport_portInterface.devPort.hwDevNum = prvTgfDevNum;
            eport_portInterface.devPort.portNum  = eportOfTrunk;
            tgfTrafficGeneratorExpectedInterfaceEport(PRV_TGF_EXPECTED_INTERFACE_TYPE_EPORT_NUM_E);
        }

        /* check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&eport_portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);
    }
    else
    {
        /* check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);
    }

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
        /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
            "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
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

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
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

    /* Restore packet's destination IP address */
    if (!matchFdb)
    {
        cpssOsMemCpy(&prvTgfPacketIpv4Part.dstAddr, &matchDstAddr, sizeof(TGF_IPV4_ADDR));
    }
}

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: get current  CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeGet(prvTgfDevNum, &prvTgfRestoreCfg.hashModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeGet");

    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 0 */
    rc = prvTgfIpPortRoutingEnableGet( PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E,
                                   &prvTgfRestoreCfg.ipv4UcPortRoutingEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet");


    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                               CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Get enable/disable status of  using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableGet( prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_IP_PROTOCOL_IPV4_E, &prvTgfRestoreCfg.fdbRoutingPortEnableOrig );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableGet");

    /* AUTODOC: Enable using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                          CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}


/**
* @internal prvTgfIpv4UcPbrRoutingPriorityFdbRoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityFdbRoutingConfigurationSet
(
    GT_U32  prvUtfVrfId
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfFdbArpMac, sizeof(TGF_MAC_ADDR));

    /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, prvTgfFdbRoutingArpIndex, &prvTgfRestoreCfg.fdbArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfFdbRoutingArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: get Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdGet(prvTgfDevNum, &prvTgfRestoreCfg.nhPacketCmdOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdGet");

    /* AUTODOC: set Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet(prvTgfDevNum, CPSS_PACKET_CMD_ROUTE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: calculate index for ipv4 uc route entry in FDB */
    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(entryKey.key.ipv4Unicast.dip, prvTgfPacketIpv4Part.dstAddr, sizeof(entryKey.key.ipv4Unicast.dip));
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
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_NEXTHOP_VLAN_ID_CNS;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfFdbRoutingArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    prvTgfFdbIpv4UcRoutingRouteConfigurationSet(GT_TRUE, &macEntry);
}

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet function
* @endinternal
*
* @brief   Set LPM leaf entry
*
* @param[in] priority              - LPM leaf entry priority
*
*/
GT_STATUS prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet
(
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority
)
{
    GT_STATUS rc;
    PRV_TGF_LPM_LEAF_ENTRY_STC              leafEntry;

    cpssOsMemSet(&leafEntry, 0, sizeof(PRV_TGF_LPM_LEAF_ENTRY_STC));

    /* write a leaf entry to the PBR area in RAM */
    leafEntry.entryType                = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    leafEntry.index                    = prvTgfRouteEntryBaseIndex;
    leafEntry.ucRPFCheckEnable         = GT_FALSE;
    leafEntry.sipSaCheckMismatchEnable = GT_FALSE;
    leafEntry.ipv6MCGroupScopeLevel    = 0;
    leafEntry.priority                 = priority;
    /* AUTODOC: Write a PBR leaf */
    rc = prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfLpmLeafEntryWrite");

    return rc;
}

/**
* @internal prvTgfIpv4UcPbrRoutingUnicastRouteForPbrEnable function
* @endinternal
*
* @brief   Enable FDB unicast routing for PBR
*
* @param[in] enable - enable value
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingUnicastRouteForPbrEnable
(
    GT_BOOL enable
)
{
    GT_STATUS rc;

    rc = prvTgfIpFdbUnicastRouteForPbrEnableSet(prvTgfDevNum, enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpFdbUnicastRouteForPbrEnableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_STATUS prvTgfIpv4UcPbrRoutingPriorityLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    GT_U32                                  portIter;

    /* clear also counters at end of test */
    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevsArray[portIter],
                                    prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_GT_OK(rc,"prvTgfResetCountersEth");
    }

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfPbrArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write ARP MAC 00:12:23:45:67:89 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, prvTgfFdbRoutingArpIndex, &prvTgfRestoreCfg.pbrArpMacAddrOrig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpRouterArpAddrRead");

    /* AUTODOC: write ARP MAC 00:12:23:45:67:89 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpRouterArpAddrWrite");

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = 0;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOP_VLAN_ID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entry 8 with nexthop VLAN 1 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpUcRouteEntriesWrite");

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpUcRouteEntriesRead");

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 2. Create a Leaf Entry
     */
    rc = prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet");

    /* Save the enabling status of bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.pbrBypassTriggerOrig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet");

    /* Enable bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.fdbUnicastRouteForPbrOrig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpFdbUnicastRouteForPbrEnableGet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableSet(prvTgfDevNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpFdbUnicastRouteForPbrEnableSet");

    return rc;
}

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityPclConfigurationSet function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS      rc = GT_OK;

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* define mask, pattern and action */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassIngressPipe                                = GT_FALSE;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
    action.redirect.data.routerLttIndex                     = prvTgfPbrLeafIndex;
    /*action.redirect.vlanId                                  = PRV_TGF_NEXTHOP_VLAN_ID_CNS;*/
    rc = prvTgfIpv4UcPbrRoutingPriorityLttRouteConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpv4UcPbrRoutingPriorityLttRouteConfigurationSet: %d", prvTgfDevNum);

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
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
    /* AUTODOC:   redirect to routerLtt entry 8 */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        prvTgfPclRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclRuleIndex);
};

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfIpv4UcPbrRoutingPriorityConfigurationRestore
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    prvTgfBurstCount = PRV_TGF_BURST_COUNT_DEFAULT_CNS;/* restore value to default */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
        * 2. Restore Route Configuration
        */
    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 0 */
    rc = prvTgfIpPortRoutingEnable( PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E,
                                   prvTgfRestoreCfg.ipv4UcPortRoutingEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");


    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmVirtualRouterDel: %d",
                                        prvTgfDevNum);
    }

    /* enable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    /* restore ARP MACs*/
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &prvTgfRestoreCfg.pbrArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfFdbRoutingArpIndex, &prvTgfRestoreCfg.fdbArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum, prvTgfRestoreCfg.nhPacketCmdOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: restore enable value of FDB IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[sendPortIndex],
                                          CPSS_IP_PROTOCOL_IPV4_E, prvTgfRestoreCfg.fdbRoutingPortEnableOrig);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* delete port from both VLANs (support route from trunk A to trunk A) */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* delete Ports from VLANs */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                        prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                         prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                         prvTgfPortsArray[portIter]);
        }

        /* delete VLANs */

        /* AUTODOC: invalidate vlan entries 5,6 */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* Restore FDB routing configuration */

    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,prvTgfRestoreCfg.hashModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* Restore PBR configuration */

    /* Enable bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, prvTgfRestoreCfg.pbrBypassTriggerOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableSet(prvTgfDevNum, prvTgfRestoreCfg.fdbUnicastRouteForPbrOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbUnicastRouteForPbrEnableSet");

    /* Restore PCL configuration */
    rc = prvTgfPclRuleValidStatusSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    prvTgfPclRestore();

    /* restore default VLAN 1 */
    prvTgfBrgVlanEntryRestore(1);
}
