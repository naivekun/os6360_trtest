/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tgfTunnelStartGenericIpv4Template.c
*
* DESCRIPTION:
*       Tunnel Start: Generic IPv4 with UDP protocol and template - Basic
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>
#include <tunnel/tgfTunnelStartGenericIpv4UdpTemplate.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS 3

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x54

/* mac SA index in global mac sa table */
#define PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS          7
/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 3;

/* flag to indecate mac entry state */
/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32        prvTgfRouterArpTunnelStartLineIndex = 8;
static GT_U16       udpDstPort = 14519;
static GT_U16       udpSrcPort = 64212;

/*  IPv4 packet to send */
static TGF_PACKET_STC prvTgfIpv4PacketInfo;

/*  IPv4 0ver IPv4 UDP Template packet to compare */
static TGF_PACKET_STC prvTgfGenericIpv4TemplatePacketInfo;
/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x88, 0x77, 0x11, 0x11, 0x55, 0x66};

/* Tunnel destination IP */
static TGF_IPV4_ADDR prvTgfTunnelDstIp = { 1,  1,  1,  3};

/* Tunnel source IP */
static TGF_IPV4_ADDR prvTgfTunnelSrcIp = { 2,  2,  2,  2};

/* type of the tunnel */
static CPSS_TUNNEL_TYPE_ENT prvTgfTunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelStartGenericIpv4TemplateBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateBridgeConfigSet
(
     GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0, 0};
    TGF_PACKET_L2_STC *l2PartPtr;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* add tags */
    tagArray[0] = (GT_U8)GT_FALSE;
    tagArray[1] = (GT_U8)GT_FALSE;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged\tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged\tagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);


    /* check is mac entry created */

    /* use L2 part */
    l2PartPtr = prvTgfIpv4PacketInfo.partsArray[0].partPtr;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, vid 5 */
    rc = prvTgfBrgDefFdbMacEntryOnVidSet(l2PartPtr->daMac,
                                         PRV_TGF_VLANID_5_CNS, PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* MAC entry successfully created */
    
}




/**
* @internal prvTgfTunnelStartGenericIpv4TemplateRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateRouteConfigurationSet

(
    GT_VOID
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;
    PRV_TGF_IP_ROUTE_CNFG_STC routeConfig;

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &prvTgfIpv4PacketInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
    /* clear entry */
    cpssOsMemSet( &routeConfig, 0, sizeof(routeConfig));

    routeConfig.devNum = prvTgfDevNum;
    cpssOsMemCpy(routeConfig.dstIpAddr,ipv4PartPtr->dstAddr,sizeof(ipv4PartPtr->dstAddr));
   /* routeConfig.dstIpAddr = ipv4PartPtr->dstAddr;*/
    routeConfig.isArpNeeded = GT_FALSE;
    routeConfig.isTunnelStart = GT_TRUE;
    routeConfig.nextHopPortNum = PRV_TGF_EGR_PORT_IDX_CNS;
    routeConfig.nextHopVlanId = PRV_TGF_VLANID_6_CNS;
    routeConfig.sendPortNum =  PRV_TGF_SEND_PORT_IDX_CNS;
    routeConfig.sendVlanId = PRV_TGF_VLANID_5_CNS;
    routeConfig.tunnelStartIndex = prvTgfRouterArpTunnelStartLineIndex;
    routeConfig.routeEntryBaseIndex = 5;
    routeConfig.countSet = PRV_TGF_COUNT_SET_CNS;
    switch(routingMode)
    {
    case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            prvTgfIpv4PbrConfigurationSet(&routeConfig);
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            prvTgfIpv4LttRouteConfigurationSet(&routeConfig);
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}


/**
* @internal prvTgfTunnelStartGenericIpv4TemplateTunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateTunnelConfigurationSet
(
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT protocolType,
    GT_U32                                      protocolNum
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileData;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileDataGet;
    TGF_PACKET_L2_STC     *l2PartPtr;
    GT_ETHERADDR          macSa;
    GT_ETHERADDR          macSaGet;
    GT_U32                globalMacSaIndexGet;

    GT_U32 i,j;
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    cpssOsMemSet(&tunnelEntry.ipv4Cfg,0,sizeof(tunnelEntry.ipv4Cfg));
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    cpssOsMemSet(&profileDataGet,0,sizeof(profileDataGet));
    cpssOsMemSet(macSa.arEther,0,sizeof(GT_ETHERADDR));
    cpssOsMemSet(macSaGet.arEther,0,sizeof(GT_ETHERADDR));


    tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E; /* eliminate Coverity warning */
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId           = PRV_TGF_VLANID_6_CNS;
    tunnelEntry.ipv4Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.up               = 0;
    tunnelEntry.ipv4Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscp             = 0;
    tunnelEntry.ipv4Cfg.dontFragmentFlag = GT_FALSE;
    tunnelEntry.ipv4Cfg.ttl              = 33;
    tunnelEntry.ipv4Cfg.autoTunnel       = GT_FALSE;
    tunnelEntry.ipv4Cfg.autoTunnelOffset = 0;
    tunnelEntry.ipv4Cfg.cfi              = 0;
    tunnelEntry.ipv4Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol = protocolType;
    tunnelEntry.ipv4Cfg.ipProtocol       = protocolNum;
    tunnelEntry.ipv4Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv4Cfg.udpDstPort       = udpDstPort;
    tunnelEntry.ipv4Cfg.udpSrcPort       = udpSrcPort;
    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfTunnelDstIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV4 */
    /* AUTODOC:   vlanId=6, ttl=33 */
    /* AUTODOC:   DA=88:77:11:11:55:66, srcIp=2.2.3.3, dstIp=1.1.1.2 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex, prvTgfTunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /*** Get a tunnel start entry ***/
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex, &tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /*  AUTODOC: Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VLANID_6_CNS, tunnelEntry.ipv4Cfg.vlanId,
                                 "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);

    /*  AUTODOC: Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                                 tunnelEntry.ipv4Cfg.macDa.arEther[5]);

    /*  AUTODOC: configure tunnel start profile */
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E; /* 16 byte */
    profileData.udpSrcPortMode = PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E;
    profileData.hashShiftLeftBitsNumber = 3;
    for (i = 0; i < 16; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if ( (i & 0x1) == GT_TRUE)
            {
                profileData.templateDataBitsCfg[i*8+j] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
            }
            else
            {
                profileData.templateDataBitsCfg[i*8+j] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E;
            }
        }
    }
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum, PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);
    rc = prvTgfTunnelStartGenProfileTableEntryGet(prvTgfDevNum, PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileDataGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);
    rc = cpssOsMemCmp(&profileData, &profileDataGet, sizeof(PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong profile data get");


    /*  AUTODOC: configure tunnel start mac SA*/
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_L2_E, &prvTgfGenericIpv4TemplatePacketInfo, (GT_VOID *)&l2PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    cpssOsMemCpy((GT_VOID*)macSa.arEther,
                 (GT_VOID*)l2PartPtr->saMac,
                 sizeof(GT_ETHERADDR));

    /* AUTODOC: configure MAC SA to global MAC SA table into index 7*/
    rc =  prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS,&macSa);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");
    rc = prvTgfIpRouterGlobalMacSaGet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaGet: %d", prvTgfDevNum);
    rc = cpssOsMemCmp(&macSa, &macSaGet, sizeof(GT_ETHERADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong mac sa data get");

    /* AUTODOC: configure index to global MAC SA table in eport attribute table */
    rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");
    rc = prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &globalMacSaIndexGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet: %d", prvTgfDevNum);
    rc = (globalMacSaIndexGet == PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS) ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "wrong mac sa index get");
}


/**
* @internal prvTgfTunnelStartGenericIpv4TemplateBuildPacket function
* @endinternal
*
* @brief   Build packet
*
* @param[in] ip_header_protocol       - ip header protocol (IP,GRE,UDP)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateBuildPacket
(
    GT_U32 ip_header_protocol
)
{
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;

    /* AUTODOC: get default IPv4 packet */
    rc = prvTgfPacketIpv4PacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");


    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &packetInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    ipv4PartPtr->totalLen = PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS;

    /* AUTODOC: changed Ipv4 */
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,0,ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    prvTgfIpv4PacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfIpv4PacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfIpv4PacketInfo.partsArray =  packetInfo.partsArray;

    /* AUTODOC: get default Generic IPv4 UDP Template packet */
    rc = prvTgfPacketGenericIpv4TemplateDefaultPacketGet(ip_header_protocol,&prvTgfGenericIpv4TemplatePacketInfo.numOfParts, &prvTgfGenericIpv4TemplatePacketInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketGenericIpv4UdpTemplateDefaultPacketGet");
    prvTgfGenericIpv4TemplatePacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
}

/**
* @internal prvTgfTunnelStartGenericIpv4TemplateCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*/
void prvTgfTunnelStartGenericIpv4TemplateCheckCaptureEgressTrafficOnPort
(
 GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC expectedPacketInfo;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;
    GT_U32  byteNumMaskList[] = { 22, 23, 28, 29 }; /* list of bytes for which
                                                       the comparison is prohibited */

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);
    /* build expected packet */

    /* copy the expected packet from the ingress packet */
    expectedPacketInfo.numOfParts = prvTgfGenericIpv4TemplatePacketInfo.numOfParts;
    expectedPacketInfo.totalLen = prvTgfGenericIpv4TemplatePacketInfo.totalLen;
    /* allocate proper memory for the 'expected' parts*/
    expectedPacketInfo.partsArray =
        cpssOsMalloc(expectedPacketInfo.numOfParts * sizeof(TGF_PACKET_PART_STC));

    if(expectedPacketInfo.partsArray == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_PTR,
                                     "cpssOsMalloc: failed");
        return;
    }

    /* copy expected parts from the original sent parts */
    for(ii = 0 ; ii < expectedPacketInfo.numOfParts ; ii++)
    {
        expectedPacketInfo.partsArray[ii] = prvTgfGenericIpv4TemplatePacketInfo.partsArray[ii];
    }

    /* print and check captured packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &expectedPacketInfo,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            byteNumMaskList, /* skip check of bytes in these positions */
            sizeof(byteNumMaskList)/sizeof(byteNumMaskList[0]), /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* free the dynamic allocated memory */
    cpssOsFree(expectedPacketInfo.partsArray);
}


/**
* @internal prvTgfTunnelStartGenericIpv4TemplateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 3  IPv4  packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */

    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfIpv4PacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    /* check captured egress traffic on port */
    prvTgfTunnelStartGenericIpv4TemplateCheckCaptureEgressTrafficOnPort();

    return;
}

/**
* @internal prvTgfTunnelStartGenericIpv4TemplateConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartGenericIpv4TemplateConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;
    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &prvTgfIpv4PacketInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP,ipv4PartPtr->dstAddr,sizeof(ipv4PartPtr->dstAddr));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(0, 0, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {
        /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
        rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_5_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* restore 'hard drop' */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_IPV4_HEADER_ERROR_E, CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    prvTgfPacketRestoreDefaultParameters();
}
/**
* @internal prvTgfTunnelStartGenericIpv4TemplateTest function
* @endinternal
*
* @brief   Run generic ipv4 tunneling test
*
* @param[in] protocolType             - tunnel start ip header protocol type
* @param[in] protocolNum              - tunnel start ip header protocol number (relevant only in case that protocolType is  PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E)
*                                       None
*/

GT_VOID prvTgfTunnelStartGenericIpv4TemplateTest
(
    PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_ENT protocolType,
    GT_U32                                      protocolNum
)

{
/*
    1. Set Base configuration
    2. Set Route configuration
    3. Set Tunnel configuration
    4. Generate traffic
    5. Restore configuration
*/
    GT_U32      notAppFamilyBmp;

    if(GT_FALSE == prvTgfBrgTtiCheck())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* NOTE: Test is wrong for PBR mode!!!
             can't do tunnel start and routing though PCL in PBR mode,
             in "IPCL Action as Next Hop Entry" there is no support for tunnel start */
    if (prvUtfIsPbrModeUsed())
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* Supported by eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    /* Build Packet */
    prvTgfTunnelStartGenericIpv4TemplateBuildPacket(protocolType);

    if(protocolType == PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E)
    {
       prvTgfPacketGenericIpvTemplateSetGenericProtocol(protocolNum,GT_FALSE); 
    }
    
    /* Set Bridge configuration*/
    prvTgfTunnelStartGenericIpv4TemplateBridgeConfigSet();

    /* Set Route configuration */
    prvTgfTunnelStartGenericIpv4TemplateRouteConfigurationSet();

    /* Set Tunnel configuration */
    prvTgfTunnelStartGenericIpv4TemplateTunnelConfigurationSet(protocolType,protocolNum);

    /* Generate traffic */
    prvTgfTunnelStartGenericIpv4TemplateTrafficGenerate();

    /* Restore configuration */
    prvTgfTunnelStartGenericIpv4TemplateConfigurationRestore();

    prvTgfPclRestore();
}




