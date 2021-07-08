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
* @file prvTgfFdbBasicIpv4UcRouting.c
*
* @brief FDB IPv4 UC Routing
*
* @version   7
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

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS   3

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET1_E

/* the Virtual Router index */
static GT_U32        prvUtfVrfId  = 1;
static GT_U32        lpmDbId      = 0;

/* default next hop packet command used for restore */
static CPSS_PACKET_CMD_ENT              prvTgfNhPacketCmdGet;


/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 0;
static GT_U32        prvTgfRouteEntryBaseIndex = 60;
static GT_U32        prvTgfEcmpEntryBaseIndex  = 89;
/* the Number of paths for the Router Table */
static GT_U32        prvTgfNumOfPaths          = 64;

static GT_ETHERADDR prvTgfArpMacAddrGet;

/* Restored values */
static GT_U32        nextHopIndexRestore[64];
static GT_BOOL       indirectAcessEnableRestore = GT_FALSE;

/******************************* Test packet **********************************/
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

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x11, 0x22, 0x00};

/**
* @internal prvTgfFdbIpv4UcPointerRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcPointerRouteConfigurationSet
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

    macEntry.fdbRoutingInfo.routingType = CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E;
    macEntry.fdbRoutingInfo.multipathPointer = prvTgfEcmpEntryBaseIndex;

    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

    prvTgfFdbIpv4UcRoutingRouteConfigurationSet(routingByIndex,&macEntry);
}

/**
* @internal prvTgfFdbIpv4UcPointerRouteConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv4UcPointerRouteConfigurationRestore
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
}

/**
* @internal prvTgfFdbIpv4UcPointerRouteLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
static GT_VOID prvTgfFdbIpv4UcPointerRouteLttRouteConfigurationSet
(
    GT_BOOL randomEnable,
    GT_BOOL indirectAcessEnable
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  pathIndex;
    PRV_TGF_IP_ECMP_ENTRY_STC               ecmpEntry;
    GT_U32                                  nextHopIndexWrite;
    GT_U32                                  nextHopIndexRead;
    GT_U32                                  lastNextHopIndexWrite = prvTgfRouteEntryBaseIndex;

    PRV_UTF_LOG2_MAC("==== Setting LTT Route Configuration ==== \n randomEnable [%d] indirectAcessEnable[%d]\n",randomEnable,indirectAcessEnable);

    /* define max number of paths */
    if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                  UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_PUMA_E))
        prvTgfNumOfPaths = 64;
    else

        prvTgfNumOfPaths = 8;

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable Trunk Hash Ip Mode */
    rc = prvTgfTrunkHashIpModeSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashIpModeSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* Create prvTgfNumOfPaths route enties (8 for xCat/Puma or 64 for Lion)
     * The rules are following:
     *   pathIndex          = [0...prvTgfNumOfPaths-1]
     *   baseRouteEntryIndex = prvTgfRouteEntryBaseIndex + pathIndex
     *   nextHopARPPointer   = prvTgfRouterArpIndex   + pathIndex
     *   countSet            = pathIndex % 4
     * IP packets will use following rules:
     *   sip   = sipBase   + pathIndex;
     *   chsum = chsumBase - pathIndex;
     *   daMac = daMacBase + pathIndex;
     */
    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
    {
        /* write a ARP MAC address to the Router ARP Table */
        prvTgfArpMac[5] = (GT_U8) (prvTgfRouterArpIndex + pathIndex);

        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        /* AUTODOC: write 8 ARP MACs 00:00:00:11:22:[01..08]  to the Router ARP Table */
        rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex + pathIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* setup default values for the route entries */
        cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
        regularEntryPtr = &routeEntriesArray[0];
        regularEntryPtr->cmd               = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
        regularEntryPtr->nextHopVlanId     = PRV_TGF_NEXTHOPE_VLANID_CNS;
        regularEntryPtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        regularEntryPtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
        regularEntryPtr->nextHopInterface.devPort.portNum =
                            prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

        /* setup the route entries by incremental values */
        regularEntryPtr->countSet          = (CPSS_IP_CNT_SET_ENT) pathIndex % 4;
        regularEntryPtr->nextHopARPPointer = prvTgfRouterArpIndex + pathIndex;

        nextHopIndexWrite =  prvTgfRouteEntryBaseIndex + pathIndex;

        if(indirectAcessEnable == GT_TRUE)
        {
            /*scatter the next hops*/
            if( (pathIndex%3) == 0)
            {
                nextHopIndexWrite = lastNextHopIndexWrite + 2;
            }
            else if( (pathIndex%3) == 1)
            {
                nextHopIndexWrite= lastNextHopIndexWrite+3;
            }
            else
            {
                nextHopIndexWrite = lastNextHopIndexWrite+4;
            }

            lastNextHopIndexWrite =  nextHopIndexWrite;

            indirectAcessEnableRestore = GT_TRUE;

            /* AUTODOC: save ECPM indirect next hop entry */
            rc = cpssDxChIpEcmpIndirectNextHopEntryGet(prvTgfDevNum,prvTgfRouteEntryBaseIndex + pathIndex,&nextHopIndexRestore[pathIndex]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntryGet: %d", prvTgfDevNum);

            rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum,prvTgfRouteEntryBaseIndex + pathIndex,nextHopIndexWrite);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntrySet: %d", prvTgfDevNum);

            rc = cpssDxChIpEcmpIndirectNextHopEntryGet(prvTgfDevNum,prvTgfRouteEntryBaseIndex + pathIndex,&nextHopIndexRead);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntryGet: %d", prvTgfDevNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(nextHopIndexWrite,nextHopIndexRead, "nextHopIndexWrite is not equal to nextHopIndexRead!Should be : %d", nextHopIndexWrite);

            PRV_UTF_LOG2_MAC("==== Indirect ecmp indirect  entry [%d] point to [%d] ====\n",prvTgfRouteEntryBaseIndex + pathIndex,nextHopIndexWrite);

        }

        /* AUTODOC: add 8 UC route entries with nexthop VLAN 6 and nexthop port 3 */
        rc = prvTgfIpUcRouteEntriesWrite(nextHopIndexWrite,
                                         routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: read and check the UC Route entry from the Route Table */
        cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, nextHopIndexWrite,
                                        routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthop info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_MULTIPATH_ROUTE_ENTRY_GROUP_E;
    }
    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) == GT_TRUE) ||
        (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_TRUE) )
    {
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfEcmpEntryBaseIndex;
        ecmpEntry.numOfPaths = prvTgfNumOfPaths;
        ecmpEntry.randomEnable = randomEnable;
        ecmpEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
        ecmpEntry.multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
        rc = prvTgfIpEcmpEntryWrite(prvTgfDevNum, prvTgfEcmpEntryBaseIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpEntryWrite: %d", prvTgfDevNum);
    }
    else
    {
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
        nextHopInfo.ipLttEntry.numOfPaths          = prvTgfNumOfPaths - 1;
    }
}

/**
* @internal prvTgfFdbIpv4UcPointerRouteEcmpRoutingConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfFdbIpv4UcPointerRouteEcmpRoutingConfigurationSet
(
    GT_BOOL randomEnable,
    GT_BOOL indirectAcessEnable
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            PRV_UTF_LOG0_MAC("==== PBR mode does not supported ====\n");
            break;
        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            prvTgfFdbIpv4UcPointerRouteLttRouteConfigurationSet(randomEnable,indirectAcessEnable);
            break;
        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:
            break;
    }
}