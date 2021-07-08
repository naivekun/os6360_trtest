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
* @file cstIpBasicIpv4UcRouting.c
*
* @brief Basic IPV4 UC Routing
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#ifdef CHX_FAMILY
#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <common/customer/cstCommon.h>
#include <common/customer/cstBridge.h>
#include <common/customer/cstIp.h>
#include <common/tgfIpGen.h>
#include <ip/customer/cstIpBasicIpv4UcRouting.h>
#include <common/tgfPacketGen.h>
#include <gtOs/gtOsMem.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>

static GT_U32   errorLine = 0;

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

#define DSA_SRC_DEV_CNS 0x1a
#define DSA_TRG_DEV_CNS 0x16
#define DSA_TRG_PORT_CNS 0x13

/* used ports */
static  GT_U32    portsArray[] = {0, 18, 36, 58};

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U8         vlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        lpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};
static TGF_MAC_ADDR  prvTgfArpMac1 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

static GT_IPADDR     ipAddrArray[3];





#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/* mac SA index in global mac sa table */
#define PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS          7

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* value of MacSa LSB*/
#define PRV_TGF_MAC_SA_LSB_CNS 0xE5

/* value of Base MacSa */
#define PRV_TGF_MAC_SA_BASE_CNS {0x00, 0xA1, 0xB2, 0xC3, 0xD4, 0x00}

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
/* packet's IPv4 - used for bulk tests */
static TGF_PACKET_IPV4_STC prvTgfPacket2Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 3,  2,  2,  3}    /* dstAddr */
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

#define VLAN_TAG_INFO_INDEX  1  /* index in prvTgfPacketPartArray[],prvTgfPacket2PartArray[]*/
#define DSA_INFO_INDEX  2       /* index in prvTgfPacketPartArray[],prvTgfPacket2PartArray[]*/

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet - used for bulk tests */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket2Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

static GT_U8    devNum = 0;
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvCstIpBasicIpv4UcBrgConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] devNum                   - device number
* @param[in] fdbPortNum               - the FDB port num to set in the macEntry destination Interface
*                                       whether the configurations passed or failed
*/
static GT_STATUS prvCstIpBasicIpv4UcBrgConfigurationSet
(
    GT_U8   devNum,
    GT_U32   fdbPortNum
)
{

    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {0, 0, 1, 1};
    CPSS_MAC_ENTRY_EXT_STC   macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = cstBrgDefVlanEntryWithPortsSet(devNum,PRV_TGF_SEND_VLANID_CNS,
            portsArray, NULL, tagArray, 2);
    CHECK_RC_MAC(rc);
    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = cstBrgDefVlanEntryWithPortsSet(devNum, PRV_TGF_NEXTHOPE_VLANID_CNS,
            portsArray + 2, NULL, tagArray + 2, 2);
    CHECK_RC_MAC(rc);

    /* AUTODOC: enable VLAN based MAC learning */
    rc = cpssDxChBrgVlanLearningStateSet(devNum, PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    CHECK_RC_MAC(rc);

    /* -------------------------------------------------------------------------
     * Set the FDB entry With DA_ROUTE
     */

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = devNum;
    macEntry.dstInterface.devPort.portNum   = fdbPortNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceID                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = cpssDxChBrgFdbMacEntrySet(devNum,&macEntry);
    CHECK_RC_MAC(rc);
    return GT_OK;
}

/**
* @internal prvCstIpBasicIpv4UcRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] devNum                   - device number
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*                                      useBulk         - whether to use bulk operation to add prefixes
*                                       whether the configurations passed or failed
*/
static GT_STATUS prvCstIpBasicIpv4UcRoutingRouteConfigurationSet
(
    GT_U8    devNum,
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32    nextHopPortNum
)
{
    GT_STATUS                   rc = GT_OK;

    GT_IPADDR                               prvIpAddr;
    GT_ETHERADDR                            arpMacAddr;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC         routeEntriesArray[1];
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC         *regularEntryPtr;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_U32                                  ii;
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC          vrConfig;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT      memoryCfg;
    GT_ETHERADDR                            macSaEntry;


    if (sendPortIndex > 3)
    {
        return GT_BAD_PARAM;
    }

    targetHwDevice = devNum;
    targetPort = nextHopPortNum;

    /* remove LPM DB and vrfId configurated in appDemo */
    cstIpCleanAppDemoConfig();

    /* AUTODOC: create LPM DB */
    rc = cpssDxChIpLpmDBCreate(lpmDBId, CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E,
                               CPSS_IP_PROTOCOL_IPV4_E, &memoryCfg);
    CHECK_RC_MAC(rc);

    /* AUTODOC: add device to LPM DB */
    rc = cpssDxChIpLpmDBDevListAdd(lpmDBId, &devNum, 1);
    CHECK_RC_MAC(rc);

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

    /* AUTODOC: add virtual router */
    rc = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, prvUtfVrfId, &vrConfig);
    CHECK_RC_MAC(rc);

    /* AUTODOC: global routing enable */
    rc =  cpssDxChIpRoutingEnable(devNum,GT_TRUE);
    CHECK_RC_MAC(rc);

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = cpssDxChIpPortRoutingEnable(devNum, portsArray[sendPortIndex], CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    CHECK_RC_MAC(rc);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = cpssDxChBrgVlanVrfIdSet(devNum, PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    CHECK_RC_MAC(rc);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = cpssDxChBrgVlanIpUcRouteEnable(devNum, PRV_TGF_SEND_VLANID_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    CHECK_RC_MAC(rc);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));
    rc = cpssDxChIpRouterArpAddrWrite(devNum, prvTgfRouterArpIndex, &arpMacAddr);
    CHECK_RC_MAC(rc);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
    regularEntryPtr->entry.regularEntry.cmd = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->entry.regularEntry.cpuCodeIdx                 = 0;
    regularEntryPtr->entry.regularEntry.appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->entry.regularEntry.unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->entry.regularEntry.ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->entry.regularEntry.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->entry.regularEntry.ingressMirror              = GT_FALSE;
    regularEntryPtr->entry.regularEntry.qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->entry.regularEntry.qosProfileIndex            = 0;
    regularEntryPtr->entry.regularEntry.qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->entry.regularEntry.modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->entry.regularEntry.modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->entry.regularEntry.countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->entry.regularEntry.trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->entry.regularEntry.sipAccessLevel             = 0;
    regularEntryPtr->entry.regularEntry.dipAccessLevel             = 0;
    regularEntryPtr->entry.regularEntry.ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->entry.regularEntry.scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->entry.regularEntry.siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->entry.regularEntry.mtuProfileIndex            = 0;
    regularEntryPtr->entry.regularEntry.isTunnelStart              = GT_FALSE;
    regularEntryPtr->entry.regularEntry.nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->entry.regularEntry.nextHopInterface.type      = CPSS_INTERFACE_PORT_E;

    rc = cpssDxChCfgHwDevNumGet((GT_U8)devNum, &targetHwDevice);
    CHECK_RC_MAC(rc);
    regularEntryPtr->entry.regularEntry.nextHopInterface.devPort.hwDevNum = targetHwDevice;

    regularEntryPtr->entry.regularEntry.nextHopInterface.devPort.portNum = targetPort;
    regularEntryPtr->entry.regularEntry.nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->entry.regularEntry.nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = cpssDxChIpUcRouteEntriesWrite(devNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    CHECK_RC_MAC(rc);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].entry.regularEntry.nextHopVlanId,
                     routeEntriesArray[0].entry.regularEntry.nextHopInterface.devPort.portNum);


    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* AUTODOC: set 3 destination IP addresses: 1.1.1.3, 1.1.1.4, 1.1.1.5  */
    prvIpAddr.u32Ip = 0x03010101;
    for (ii = 0; ii < 3; ii++)
    {
        ipAddrArray[ii].u32Ip = prvIpAddr.u32Ip;
        rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, prvUtfVrfId, &(ipAddrArray[ii]), 32, &nextHopInfo, GT_FALSE,GT_FALSE);
        CHECK_RC_MAC(rc);
        prvIpAddr.arIP[3]++;
    }
    /* AUTODOC: configure MAC SA on port egrPort */

    /* AUTODOC: enable MAC SA modification on port egrPort */
    rc = cpssDxChIpRouterMacSaModifyEnable(devNum, nextHopPortNum, GT_TRUE);
    CHECK_RC_MAC(rc);

    /* AUTODOC: set MAC SA LSB mode = CPSS_SA_LSB_PER_PKT_VID_E  */
    /* Sets the MAC SA least significant 12-bits according to the packet's VLAN ID. */
    /* The MAC SA is based on the globally configured Router MAC Address where the 12 least
       significant bits are set according to the outer tag VLAN-ID. */
    rc = cpssDxChIpPortRouterMacSaLsbModeSet(devNum, nextHopPortNum, CPSS_SA_LSB_PER_PKT_VID_E);
    CHECK_RC_MAC(rc);
    macSaEntry.arEther[0] = 0x03;
    macSaEntry.arEther[1] = 0x0c;
    macSaEntry.arEther[2] = 0x0b;
    macSaEntry.arEther[3] = 0x0a;
    macSaEntry.arEther[4] = 0x02;
    macSaEntry.arEther[5] = 0x01;
    /* AUTODOC: configure the 40 most significant bits of the MAC SA base address */
    rc = cpssDxChIpRouterMacSaBaseSet(devNum,&macSaEntry);
    CHECK_RC_MAC(rc);

    return GT_OK;
}
#endif  /* CHX_FAMILY */

/**
* @internal cstIpBasicIpv4UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID cstIpBasicIpv4UcRoutingTrafficGenerate
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
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
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    GT_U32      numVfd = 0;/* number of VFDs in vfdArray */
    GT_U32       egressPortIndex = 0xFF;
    GT_BOOL      isRoutedPacket = GT_FALSE;
    GT_U32       numberOfDifferentPackets = 3;
    GT_U32       j = 0;
    GT_U32    sendPortNum = prvTgfPortsArray[0];
    GT_U32    nextHopPortNum = prvTgfPortsArray[3];
    GT_BOOL  useSecondPkt = GT_FALSE;
    GT_BOOL  useSecondArp = GT_FALSE;
    GT_BOOL  isUrpfTest = GT_FALSE;
    GT_BOOL  expectNoTraffic = GT_FALSE;


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

    for (j = 0; j < numberOfDifferentPackets; j++)
    {
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

        nextHopTrunkId = 0;

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = nextHopPortNum;

        captureType = TGF_CAPTURE_MODE_MIRRORING_E;


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

        /* number of parts in packet */
        if (useSecondPkt == GT_TRUE)
        {
            partsCount = sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]);
            packetPartsPtr = prvTgfPacket2PartArray;
        }
        else
        {
            partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
            packetPartsPtr = prvTgfPacketPartArray;
        }
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
        prvTgfPacketIpv4Part.dstAddr[3]++;

        /* -------------------------------------------------------------------------
         * 3. Get Ethernet Counters
         */

        /* read and check ethernet counters */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            GT_BOOL isOk;


            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);


            /* AUTODOC: verify routed packet on port 3 */
            if (isUrpfTest == GT_TRUE)
            {
                if ( (expectNoTraffic == GT_TRUE && prvTgfPortsArray[portIter]==sendPortNum) ||
                     (expectNoTraffic == GT_FALSE && ((prvTgfPortsArray[portIter]==sendPortNum) || (portIter ==egressPortIndex))) )
                {
                    isRoutedPacket = GT_TRUE;
                }
            }
            else
            {   if ( (prvTgfPortsArray[portIter]==sendPortNum) ||
                     ((expectNoTraffic==GT_FALSE)&&(portIter ==egressPortIndex)))
                {
                    isRoutedPacket = GT_TRUE;
                }
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

        if (expectNoTraffic == GT_TRUE && isUrpfTest == GT_TRUE)
        {
            return;
        }

        /* -------------------------------------------------------------------------
         * 4. Get Trigger Counters
         */

        if(expectNoTraffic==GT_FALSE)
        {
            PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

            /* get trigger counters where packet has MAC DA as prvTgfArpMac */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 0;
            vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
            if (useSecondArp)
            {
                cpssOsMemCpy(arpMacAddr, &prvTgfArpMac1, sizeof(TGF_MAC_ADDR));
            }
            else
            {
                cpssOsMemCpy(arpMacAddr, &prvTgfArpMac, sizeof(TGF_MAC_ADDR));
            }
            cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));

            /* disable capture on nexthope port , before check the packets to the CPU */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                         prvTgfDevNum, captureType);

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
    prvTgfPacketIpv4Part.dstAddr[3] = 3;
#else
    return;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstIpBasicIpv4UcRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_STATUS cstIpBasicIpv4UcRoutingConfigurationRestore
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_U32   prvUtfVrfId = 0;
    GT_U8    sendPortIndex = 0;
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(vlanArray)/sizeof(vlanArray[0]);
    GT_STATUS   rc        = GT_OK;
    GT_U32                  ii;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    prvTgfBurstCount = PRV_TGF_BURST_COUNT_DEFAULT_CNS;/* restore value to default */

    /* delete the FDB entry */
    rc = cstBrgFdbFlush(GT_TRUE);
    CHECK_RC_MAC(rc);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    CHECK_RC_MAC(rc);

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* AUTODOC: delete 3  Ipv4 prefix */
    for (ii = 0; ii < 3; ii++)
    {
        rc =  cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, prvUtfVrfId, &(ipAddrArray[ii]), 32);
        CHECK_RC_MAC(rc);
    }

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = cpssDxChIpPortRoutingEnable(devNum, portsArray[sendPortIndex], CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    CHECK_RC_MAC(rc);


    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = cpssDxChBrgVlanIpUcRouteEnable(devNum, PRV_TGF_SEND_VLANID_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    CHECK_RC_MAC(rc);
    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = cpssDxChIpLpmVirtualRouterDel(devNum, prvUtfVrfId);
        CHECK_RC_MAC(rc);
    }

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* delete port from both VLANs (support rout from trunk A to trunk A) */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* delete Ports from VLANs */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
           rc = cpssDxChBrgVlanPortDelete(devNum, vlanArray[vlanIter], portsArray[portIter]);
           CHECK_RC_MAC(rc);
        }

        /* delete VLANs */

        /* AUTODOC: invalidate vlan entries 5,6 */
        rc = cstBrgDefVlanEntryInvalidate(devNum, vlanArray[vlanIter]);
        CHECK_RC_MAC(rc);
    }

    /* AUTODOC: disable VLAN based MAC learning */
    rc = cpssDxChBrgVlanLearningStateSet(devNum, PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    CHECK_RC_MAC(rc);
    return GT_OK;
#else
    return GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */
}

/**
* @internal cstIpBasicIpv4UcRoutingConfiguration function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS cstIpBasicIpv4UcRoutingConfiguration
(
    GT_VOID
)
{
    GT_STATUS rc;

#ifdef CHX_FAMILY
    /* to avoid compilation warnings.
       errorLine is used in CHECK_RC_MAC */
    TGF_PARAM_NOT_USED(errorLine);

    devNum = prvTgfDevNum;

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            portsArray[ii] = prvTgfPortsArray[ii];
        }
    }

    rc = prvCstIpBasicIpv4UcBrgConfigurationSet(devNum,portsArray[1]);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCstIpBasicIpv4UcRoutingRouteConfigurationSet(devNum,0,0,prvTgfPortsArray[3]);
    if (rc != GT_OK)
    {
        return rc;
    }
#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;
}

/**
* @internal cstIpBasicIpv4UcRoutingTest function
* @endinternal
*
* @brief   see file description.
*/
GT_STATUS cstIpBasicIpv4UcRoutingTest
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* perform basic L2 configurations */
    rc = cstIpBasicIpv4UcRoutingConfiguration();
    if (rc != GT_OK)
    {
        return rc;
    }

    cstIpBasicIpv4UcRoutingTrafficGenerate();

    rc = cstIpBasicIpv4UcRoutingConfigurationRestore();
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


