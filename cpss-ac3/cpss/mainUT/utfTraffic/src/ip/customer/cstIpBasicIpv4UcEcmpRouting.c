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
* @file cstIpBasicIpv4UcEcmpRouting.c
*
* @brief Basic IPV4 UC ECMP Routing
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <common/tgfIpGen.h>
#include <common/customer/cstCommon.h>
#include <common/customer/cstBridge.h>
#include <common/customer/cstIp.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_CST_INGRESS_VLAN_ID                     5

/* nextHop VLAN Id */
#define PRV_CST_NEXTHOP_VLANID_CNS                  6

/* number of ports */
#define PRV_CST_PORTS_COUNT_CNS                     4

/* port number to send traffic to */
#define PRV_CST_INGRESS_PORT_IDX_CNS                0

/* port number to send traffic to */
#define PRV_CST_FDB_PORT_IDX_CNS                    1

/* nextHop port number to receive traffic from */
#define PRV_CST_NEXTHOP_2_PORT_IDX_CNS              3

static GT_U32   errorLine = 0;

/* the LPM DB id for LPM Table */
static GT_U32        lpmDBId                    = 0;

/* the virtual router id */
static GT_U32        vrId                       = 0;

/* the Number of paths for the Router Table */
static GT_U32        numOfEcmpPaths             = 64;

/* Base Indexes for the Route, ARP, SIP and ChkSum */
static GT_U32        routeEntryBaseIndex        = 60;
static GT_U32        ecmpEntryBaseIndex         = 89;
static GT_U32        arpEntryBaseIndex          = 0;
static GT_U8         sipBaseIndex               = 1;
static GT_U8         sipStep                    = 1;

static GT_U8         swDevNum = 0;
static GT_HW_DEV_NUM hwDevNum = 0;
static GT_U32   portsArray[PRV_CST_PORTS_COUNT_CNS] = {0, 18, 36, 58};

/********************* Router ARP Table Configuration *************************/

static GT_U8      arpMac[6] = {0x00, 0x00, 0x00, 0x11, 0x22, 0x00};

/******************************* Test packet **********************************/

/* default number of packets to send */
static GT_U32        prvTgfBurstCount           = 1;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_CST_INGRESS_VLAN_ID                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
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
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Test implementation ******************************/

static GT_STATUS prvCstBridgeSet
(
    GT_VOID
)
{
    CPSS_MAC_ENTRY_EXT_STC macEntry;
    GT_U8 tagArray[] = {1, 1, 1, 1};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: get hw dev num */
    CHECK_RC_MAC(cpssDxChCfgHwDevNumGet(swDevNum, &hwDevNum));

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    CHECK_RC_MAC(cstBrgDefVlanEntryWithPortsSet(swDevNum, PRV_CST_INGRESS_VLAN_ID,
                                                portsArray, NULL, tagArray, 2));

    /* AUTODOC: create VLAN 6 with all tagged ports */
    CHECK_RC_MAC(cstBrgDefVlanEntryWithPortsSet(swDevNum, PRV_CST_NEXTHOP_VLANID_CNS,
                                                portsArray, NULL, tagArray, 4));

    /* create a static macEntry in SEND VLAN with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                 = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId        = PRV_CST_INGRESS_VLAN_ID;
    macEntry.dstInterface.type             = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = hwDevNum;
    macEntry.dstInterface.devPort.portNum  = portsArray[PRV_CST_FDB_PORT_IDX_CNS];
    macEntry.isStatic                      = GT_TRUE;
    macEntry.daRoute                       = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    CHECK_RC_MAC(cpssDxChBrgFdbMacEntrySet(swDevNum, &macEntry));

    return GT_OK;
}

static GT_STATUS prvCstRouterSet
(
    GT_VOID
)
{
    GT_ETHERADDR                            arpMacAddr;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC         routeEntriesArray[1];
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  pathIndex;
    CPSS_DXCH_IP_ECMP_ENTRY_STC             ecmpEntry;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT      memoryCfg;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC          vrConfig;
    GT_ETHERADDR                            macSaEntry;

    /* AUTODOC: global routing enable */
    CHECK_RC_MAC(cpssDxChIpRoutingEnable(swDevNum,GT_TRUE));

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    CHECK_RC_MAC(cpssDxChIpPortRoutingEnable(swDevNum,
                                             portsArray[PRV_CST_INGRESS_PORT_IDX_CNS],
                                             CPSS_IP_UNICAST_E,
                                             CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE));

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    CHECK_RC_MAC(cpssDxChBrgVlanIpUcRouteEnable(swDevNum, PRV_CST_INGRESS_VLAN_ID,
                                                CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE));

    /* AUTODOC: enable Trunk Hash Ip Mode */
    CHECK_RC_MAC(cpssDxChTrunkHashIpModeSet(swDevNum, GT_TRUE));

    /* remove LPM DB and vrfId configurated in appDemo */
    CHECK_RC_MAC(cstIpCleanAppDemoConfig());

    /* AUTODOC: create LPM DB */
    CHECK_RC_MAC(cpssDxChIpLpmDBCreate(lpmDBId, CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, &memoryCfg));

    /* AUTODOC: add device to LPM DB */
    CHECK_RC_MAC(cpssDxChIpLpmDBDevListAdd(lpmDBId, &swDevNum, 1));

    /* AUTODOC: add virtual router */
    vrConfig.supportIpv4Uc = GT_TRUE;
    vrConfig.supportIpv6Uc = GT_FALSE;
    vrConfig.supportIpv4Mc = GT_FALSE;
    vrConfig.supportIpv6Mc = GT_FALSE;
    vrConfig.supportFcoe   = GT_FALSE;
    vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vrConfig.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    vrConfig.defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    vrConfig.defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    CHECK_RC_MAC(cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, &vrConfig));

    for (pathIndex = 0; pathIndex < numOfEcmpPaths; pathIndex++)
    {
        /* write an ARP MAC address to the Router ARP Table */
        arpMac[5] = (GT_U8) (arpEntryBaseIndex + pathIndex);

        cpssOsMemCpy(arpMacAddr.arEther, arpMac, sizeof(TGF_MAC_ADDR));

        /* AUTODOC: write 8 ARP MACs 00:00:00:11:22:[01..08]  to the Router ARP Table */
        CHECK_RC_MAC(cpssDxChIpRouterArpAddrWrite(swDevNum,
                                                  arpEntryBaseIndex + pathIndex,
                                                  &arpMacAddr));

        /* setup default values for the route entries */
        cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
        routeEntriesArray[0].type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
        routeEntriesArray[0].entry.regularEntry.cmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
        routeEntriesArray[0].entry.regularEntry.nextHopVlanId = PRV_CST_NEXTHOP_VLANID_CNS;
        routeEntriesArray[0].entry.regularEntry.nextHopInterface.type = CPSS_INTERFACE_PORT_E;
        routeEntriesArray[0].entry.regularEntry.nextHopInterface.devPort.hwDevNum = hwDevNum;
        routeEntriesArray[0].entry.regularEntry.nextHopInterface.devPort.portNum =
                            portsArray[PRV_CST_NEXTHOP_2_PORT_IDX_CNS];

        /* setup the route entries by incremental values */
        routeEntriesArray[0].entry.regularEntry.countSet          = (CPSS_IP_CNT_SET_ENT) pathIndex % 4;
        routeEntriesArray[0].entry.regularEntry.nextHopARPPointer = arpEntryBaseIndex + pathIndex;

        /* AUTODOC: add 64 UC route entries with nexthop VLAN 6 and nexthop port 3 */
        CHECK_RC_MAC(cpssDxChIpUcRouteEntriesWrite(swDevNum,
                                                   routeEntryBaseIndex + pathIndex,
                                                   routeEntriesArray, 1));
    }

    /* fill ECMP info */
    ecmpEntry.numOfPaths = numOfEcmpPaths;
    ecmpEntry.randomEnable = GT_FALSE;
    ecmpEntry.routeEntryBaseIndex = routeEntryBaseIndex;

    /* AUTODOC: add ECMP entry */
    CHECK_RC_MAC(cpssDxChIpEcmpEntryWrite(swDevNum, ecmpEntryBaseIndex,
                                          &ecmpEntry));

    /* fill a nexthop info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = ecmpEntryBaseIndex;
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 and numOfPaths 7 */
    CHECK_RC_MAC(cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, 32,
                                              &nextHopInfo, GT_FALSE,GT_FALSE));
    /* AUTODOC: configure MAC SA on port egrPort */

    /* AUTODOC: enable MAC SA modification on port egrPort */
    CHECK_RC_MAC(cpssDxChIpRouterMacSaModifyEnable(swDevNum, portsArray[PRV_CST_NEXTHOP_2_PORT_IDX_CNS], GT_TRUE));

    /* AUTODOC: set MAC SA LSB mode = CPSS_SA_LSB_PER_PKT_VID_E  */
    /* Sets the MAC SA least significant 12-bits according to the packet's VLAN ID. */
    /* The MAC SA is based on the globally configured Router MAC Address where the 12 least
       significant bits are set according to the outer tag VLAN-ID. */
    CHECK_RC_MAC(cpssDxChIpPortRouterMacSaLsbModeSet(swDevNum, portsArray[PRV_CST_NEXTHOP_2_PORT_IDX_CNS], CPSS_SA_LSB_PER_PKT_VID_E));
    macSaEntry.arEther[0] = 0x03;
    macSaEntry.arEther[1] = 0x0c;
    macSaEntry.arEther[2] = 0x0b;
    macSaEntry.arEther[3] = 0x0a;
    macSaEntry.arEther[4] = 0x02;
    macSaEntry.arEther[5] = 0x01;
    /* AUTODOC: configure the 40 most significant bits of the MAC SA base address */
    CHECK_RC_MAC(cpssDxChIpRouterMacSaBaseSet(0,&macSaEntry));

    return GT_OK;
}

static GT_STATUS prvCstBridgeRestore
(
    GT_VOID
)
{
    GT_U32 portIter = 0;

    /* flush FDB include static entries */
    CHECK_RC_MAC(cstBrgFdbFlush(GT_TRUE));

    /* delete Ports 0,1 from VLAN 5 */
    for (portIter = 0; portIter < 2; portIter++)
    {
        /* delete ports from VLAN */
        CHECK_RC_MAC(cpssDxChBrgVlanPortDelete(swDevNum, PRV_CST_INGRESS_VLAN_ID, portsArray[portIter]));
    }

    /* delete Ports 0,1,2,3 from VLAN 6 */
    for (portIter = 0; portIter < PRV_CST_PORTS_COUNT_CNS; portIter++)
    {
        /* delete ports from VLAN */
        CHECK_RC_MAC(cpssDxChBrgVlanPortDelete(swDevNum, PRV_CST_NEXTHOP_VLANID_CNS, portsArray[portIter]));
    }

   /* invalidate SEND VLAN entry */
    CHECK_RC_MAC(cstBrgDefVlanEntryInvalidate(swDevNum, PRV_CST_INGRESS_VLAN_ID));

    /* invalidate NEXTHOP VLAN entry */
    CHECK_RC_MAC(cstBrgDefVlanEntryInvalidate(swDevNum, PRV_CST_NEXTHOP_VLANID_CNS));

    return GT_OK;
}

static GT_STATUS prvCstRouterRestore
(
    GT_VOID
)
{
    GT_IPADDR   ipAddr;
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntry;

    /* Clear the ECMP entry */
    ecmpEntry.numOfPaths = 1;
    ecmpEntry.randomEnable = GT_FALSE;
    ecmpEntry.routeEntryBaseIndex = 0;
    CHECK_RC_MAC(cpssDxChIpEcmpEntryWrite(swDevNum, ecmpEntryBaseIndex, &ecmpEntry));

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    CHECK_RC_MAC(cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, 32));

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    CHECK_RC_MAC(cpssDxChIpPortRoutingEnable(swDevNum, portsArray[PRV_CST_INGRESS_PORT_IDX_CNS], CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE));

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    CHECK_RC_MAC(cpssDxChBrgVlanIpUcRouteEnable(swDevNum, PRV_CST_INGRESS_VLAN_ID, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE));

    /* AUTODOC: disable Trunk Hash Ip Mode */
    CHECK_RC_MAC(cpssDxChTrunkHashIpModeSet(swDevNum, GT_FALSE));

    return GT_OK;
}
#endif  /* CHX_FAMILY */

GT_VOID cstIpBasicIpv4UcEcmpRoutingTrafficGenerate
(
    GT_BOOL randomEnable
)
{
#ifdef CHX_FAMILY
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          pathIndex;
    GT_U8                           expPktsRcv;
    GT_U8                           expPktsSent;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic for ECMP =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable capture on nexthope port */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_CST_NEXTHOP_2_PORT_IDX_CNS];

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
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 10);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    for (pathIndex = 0; pathIndex < numOfEcmpPaths; pathIndex++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture before sending the traffic */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            if ((prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS)) || (randomEnable == GT_FALSE))
            {
                /*
                 * We have to increase SIP with step calculating by diferent formules for xCat and Lion.
                 * packetHash - it is 3 last bits of SIP (when DIP is the same)
                 *
                 * xCatRange0_7:  RouteEntryIndex + packetHash % (NumOfPaths + 1)
                 * LionRange0_63: RouteEntryIndex + Floor( packetHash * (NumOfPaths + 1) / 64 )
                 */

                /* modify the packet */
                arpMac[5] = (GT_U8) ((arpEntryBaseIndex + sipBaseIndex + pathIndex)
                                            & (numOfEcmpPaths - 1));
                prvTgfPacketIpv4Part.srcAddr[3] = (GT_U8) (sipBaseIndex + pathIndex * sipStep);

                /* AUTODOC: send 8 IPv4 packets from port 0 with: */
                /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
                /* AUTODOC:   VID=5, srcIPs=2.2.2.[1..8], dstIP=1.1.1.3 */
            }
        }
        else
        {
            /*
             * The Route Entry offset within the block of ECMP entries is calculated as follows:
             * Route Entry offset = (ECMP_Trunk Hash index) modulo (number of route entries in the block)
             * ECMP_Trunk Hash index = (Trunk hash index) XOR (Source IP Address[11:6]) XOR
             * (Destination IP address[11:6])
             */

            /* modify the packet */
            arpMac[5] = (GT_U8) (arpEntryBaseIndex + pathIndex);
            sipBaseIndex        = 4;
            sipStep             = 1;
            prvTgfPacketIpv4Part.srcAddr[3] = (GT_U8) (sipBaseIndex    + pathIndex * sipStep);

            /* AUTODOC: send 8 IPv4 packets from port 0 with: */
            /* AUTODOC: DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
            /* AUTODOC: VID=5, srcIPs=2.2.2.[4.6.8.0xa.0xc.0xE.0x10,0x12], dstIP=1.1.1.3 */
        }

        /* send packet */
        PRV_UTF_LOG4_MAC("Send packet: SIP=%x, DA=%x, RouteEntry=%x, ArpEntry=%x\n",
                         prvTgfPacketIpv4Part.srcAddr[3],
                         arpMac[5],
                         routeEntryBaseIndex + pathIndex,
                         arpEntryBaseIndex + pathIndex);


        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_CST_INGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_CST_INGRESS_PORT_IDX_CNS]);

        /* AUTODOC: verify routed packet on port 3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            expPktsRcv = expPktsSent = (GT_U8) (portIter == PRV_CST_INGRESS_PORT_IDX_CNS ||
                                   portIter == PRV_CST_NEXTHOP_2_PORT_IDX_CNS);

            /* prevent "flip flopping" behaviour" */
            tgfSimSleep(5);

            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[portIter], expPktsRcv, expPktsSent,
                    prvTgfPacketInfo.totalLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        if (randomEnable == GT_TRUE)
        {
            /* in WM we don't use the pseudo random number generation, so each of
               the nexthops is legal */
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR) - sizeof(GT_U8);
        }
        else
        {
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        }
        cpssOsMemCpy(vfdArray[0].patternPtr, arpMac, sizeof(TGF_MAC_ADDR));

        /* disable capture on nexthope port - before check packets to the CPU */
        /* NOTE: this is done inside the loop , because:
           the function tgfTrafficGeneratorPortTxEthCaptureSet is general responsible
           to do needed 'sleep' to adjust the time that needed to allow the AppDemo
           to process all the packets sent to the CPU.
        */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* check TriggerCounters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

        /* AUTODOC: check if captured packet has the same MAC DA as ARP MAC */
        if (randomEnable == GT_TRUE)
        {
            UTF_VERIFY_EQUAL5_STRING_MAC(1, numTriggers,
                    "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:**\n",
                    arpMac[0], arpMac[1], arpMac[2],
                    arpMac[3], arpMac[4]);
        }
        else
        {
            UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
                    "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    arpMac[0], arpMac[1], arpMac[2],
                    arpMac[3], arpMac[4], arpMac[5]);
        }
    }

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_CST_PORTS_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters %d:\n", portIter);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
#else
    return;
#endif  /* CHX_FAMILY */
}

GT_STATUS cstIpBasicIpv4UcEcmpRoutingConfig
(
    GT_VOID
)
{
    GT_STATUS rc;

#ifdef CHX_FAMILY
   /* to avoid compilation warnings.
       errorLine is used in CHECK_RC_MAC */
    TGF_PARAM_NOT_USED(errorLine);

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            portsArray[ii] = prvTgfPortsArray[ii];
        }
    }

    rc = prvCstBridgeSet();
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCstRouterSet();
    if (rc != GT_OK)
    {
        return rc;
    }
#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;
}

GT_STATUS cstIpBasicIpv4UcEcmpRoutingRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

#ifdef CHX_FAMILY
    rc = prvCstBridgeRestore();
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCstRouterRestore();
    if (rc != GT_OK)
    {
        return rc;
    }
#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;
}

GT_STATUS cstIpBasicIpv4UcEcmpRoutingTest
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = cstIpBasicIpv4UcEcmpRoutingConfig();
    if (rc != GT_OK)
    {
        return rc;
    }

    cstIpBasicIpv4UcEcmpRoutingTrafficGenerate(GT_FALSE);

    rc = cstIpBasicIpv4UcEcmpRoutingRestore();
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


