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
* @file prvTgfTunnelStartEtherOverIpv4GreRedirectToEgress.c
*
* @brief Tunnel start Ethernet over Ipv4 GRE redirect to egress
*
* @version   13
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/cpssTypes.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <bridge/prvTgfVlanManipulation.h>
#include <tunnel/prvTgfTunnelStartEtherOverIpv4GreRedirectToEgress.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* number of vfd's */
#define PRV_TGF_VFD_NUM_CNS             6

/* pcl rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS        1

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 3;

/* expected number of packets on ports */
static GT_U8 prvTgfPacketsCountRxTxArr[PRV_TGF_PORTS_NUM_CNS] = { 1, 0, 1, 0 };

/***************** Tunnel config params  *******************************/

/* line index for the tunnel start entry in the router ARP / tunnel start table */
static GT_U32 prvTgfRouterArpTunnelStartLineIndex = 8;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x88, 0x99, 0x77, 0x66, 0x55, 0x88};


/***************** Incapsulation ethernet frame **********************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x25}     /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS              /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {0x0030};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS    \
       TGF_L2_HEADER_SIZE_CNS   + \
       TGF_VLAN_TAG_SIZE_CNS    + \
       TGF_ETHERTYPE_SIZE_CNS   + \
       sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                  /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/* indication that we check the egress port as dsa tag port */
static GT_BOOL              testEgressPortAsDsa = GT_FALSE;
/* The DSA tag type for the egress port - relevant when testEgressPortAsDsa = GT_TRUE */
static TGF_DSA_TYPE_ENT     testEgressDsaType = TGF_DSA_LAST_TYPE_E;

/* expected DSA words on egress port */
static TGF_VFD_INFO_STC     EgressCascadePort_DsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*0 /*offset*/, {0xf0,0x00,0x10,0x05,0,0},
                                         {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 4*1 /*offset*/, {0x12,0x00,0x04,0x90,0,0},
                                         {0,0,0,0,0,0}, 4/*cycleCount*/ , NULL,0,0}
};

/* expected DSA words on egress port */
static TGF_VFD_INFO_STC     EgressCascadePort_eDsaWordsVfd[4] =
{
     {TGF_VFD_MODE_STATIC_E,0,0, 12 + 8*0 /*offset*/, {0xf0,0x00,0x10,0x05,0x92,0x00,0x04,0x90},
                                                {0,0,0,0,0,0,0,0,0,0}, 8/*cycleCount*/ , NULL,0,0}
    ,{TGF_VFD_MODE_STATIC_E,0,0, 12 + 8*1 /*offset*/, {0xa0,0x00,0x00,0x00,0x02,0x00,0x12,0x00},
                                                {0,0,0,0,0,0,0,0,0,0}, 8/*cycleCount*/ , NULL,0,0}
};

/* Parameters needed to be restored */
static GT_U32                            portsArrayForRestore[PRV_TGF_MAX_PORTS_NUM_CNS];

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal tgfTunnelStartTestAsDsaTagged function
* @endinternal
*
* @brief   set the tests to be with egress DSA tag (extended DSA/eDSA) on the egress ports.
*
* @param[in] egressDsaType            - type of DSA tag
*                                       None
*/
GT_STATUS tgfTunnelStartTestAsDsaTagged
(
    TGF_DSA_TYPE_ENT    egressDsaType
)
{
    testEgressPortAsDsa = GT_TRUE;
    testEgressDsaType   = egressDsaType;
    return GT_OK;
}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_HW_DEV_NUM hwDevNum;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(portsArrayForRestore, prvTgfPortsArray,
                 sizeof(portsArrayForRestore));
    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        GT_STATUS   rc;
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_EGR_PORT_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_OK, "prvTgfPortsArray rearrangement can't be done");
    }



    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, NULL, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* set default vlan entry for vlan ID 6 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_VLANID_5_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, hwDevNum);

    /* set hwDevNum in expected DSA tag */
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(EgressCascadePort_DsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(EgressCascadePort_DsaWordsVfd[1].patternPtr[3], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(EgressCascadePort_eDsaWordsVfd[0].patternPtr[0], hwDevNum);
    PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(EgressCascadePort_eDsaWordsVfd[0].patternPtr[7], hwDevNum);
}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*
* @param[in] retainCrc                - determines whether the original passenger Ethernet CRC is
*                                      retain or stripped
*                                       None
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTunnelConfigSet
(
    IN GT_BOOL      retainCrc
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    /* clear tunnelEntry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
    }

    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable    = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId       = PRV_TGF_VLANID_5_CNS;
    tunnelEntry.ipv4Cfg.upMarkMode   = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscpMarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.ttl          = 40;
    tunnelEntry.ipv4Cfg.ethType      = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.retainCrc    = retainCrc;
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        if (retainCrc)
        {
            tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
        }
        else
        {
            tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
        }
    }

    cpssOsMemSet((GT_VOID*) &tunnelEntry.ipv4Cfg.srcIp.arIP, 0xA,
                 sizeof(tunnelEntry.ipv4Cfg.srcIp.arIP));
    cpssOsMemSet((GT_VOID*) &tunnelEntry.ipv4Cfg.destIp.arIP, 4,
                 sizeof(tunnelEntry.ipv4Cfg.destIp.arIP));

    /* tunnel next hop MAC DA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther,
                 prvTgfTunnelMacDa, sizeof(prvTgfTunnelMacDa));

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=X_OVER_GRE_IPV4 */
    /* AUTODOC:   vlanId=5, ttl=40, ethType=TUNNEL_GRE0_ETHER_TYPE */
    /* AUTODOC:   upMarkMode=dscpMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   DA=88:99:77:66:55:88, srcIp=10.10.10.10, dstIp=4.4.4.4 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex,
                                   tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* clear tunnelEntry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex,
                                   &tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VLANID_5_CNS, tunnelEntry.ipv4Cfg.vlanId,
                     "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa,
                      sizeof(prvTgfTunnelMacDa)) == 0 ?  GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                            tunnelEntry.ipv4Cfg.macDa.arEther[5]);
}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSet function
* @endinternal
*
* @brief   Set Pcl configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, LOOKUP_0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /******* set PCL rule - FORWARD packet with MAC DA = ... 34 02 *******/
    /* clear mask */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* clear pattern */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* clear action */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* setup action */
    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    action.redirect.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;

    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType =
                            PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr =
                                            prvTgfRouterArpTunnelStartLineIndex;

    /* setup pattern */
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(pattern.ruleStdNotIp.macDa.arEther));

    /* setup mask */
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleStdNotIp.macDa.arEther));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   redirectCmd=REDIRECT_CMD_OUT_IF, port=2  */
    /* AUTODOC:   outlifType=TUNNEL, tunnelType=ETHERNET, tsPtr=8 */
    /* AUTODOC:   pattern DA=00:00:00:00:34:02 */
    rc = prvTgfPclRuleSet(ruleFormat, PRV_TGF_PCL_RULE_IDX_CNS,
                          &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 prvTgfDevNum, ruleFormat);
}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] retainCrc                - determines whether the original passenger Ethernet CRC is
*                                      retain or stripped
*                                       None
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressTrafficGenerate
(
    IN GT_BOOL  retainCrc
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter    = 0;
    GT_U32    packetSize  = 0;
    GT_U32    packetIter  = 0;
    GT_U32    numTriggers = 0;
    GT_U32    triggerIndex;
    GT_U32    retainCrcNumBytes;
    GT_U32                          sizeDSATag = 0;
    TGF_VFD_INFO_STC                vfdArray[10];
    TGF_VFD_INFO_STC                *currentDsaWordsVfdPtr;
    CPSS_INTERFACE_INFO_STC         portInterface;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
    /* the test with retainCrcNumBytes == GT_TRUE , is not using GRE */
    /* so the addition 4 bytes of CRC replaced with the 4 bytes of the GRE that not exists */
    retainCrcNumBytes = 0;

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];


    /* check da mac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset     = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));

    /* check vlan ethertype */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].offset     = sizeof(prvTgfTunnelMacDa) * 2;
    vfdArray[1].cycleCount = TGF_ETHERTYPE_SIZE_CNS;
    vfdArray[1].patternPtr[0] = TGF_ETHERTYPE_8100_VLAN_TAG_CNS >> 8;
    vfdArray[1].patternPtr[1] = TGF_ETHERTYPE_8100_VLAN_TAG_CNS & 0xFF;

    /* check vlan id */
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].offset     = vfdArray[1].offset + vfdArray[1].cycleCount;
    vfdArray[2].cycleCount = 2;
    vfdArray[2].patternPtr[0] = 0;
    vfdArray[2].patternPtr[1] = PRV_TGF_VLANID_5_CNS;

    /* check ethertype */
    vfdArray[3].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[3].offset     = vfdArray[2].offset + vfdArray[2].cycleCount;
    vfdArray[3].cycleCount = 2;
    vfdArray[3].patternPtr[0] = TGF_ETHERTYPE_0800_IPV4_TAG_CNS >> 8;
    vfdArray[3].patternPtr[1] = TGF_ETHERTYPE_0800_IPV4_TAG_CNS & 0xFF;

    /* check ipv4 header (version) */
    vfdArray[4].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[4].offset     = vfdArray[3].offset + vfdArray[3].cycleCount;
    vfdArray[4].cycleCount = 2;
    vfdArray[4].patternPtr[0] = 0x45;
    vfdArray[4].patternPtr[1] = 0;

    /* check ipv4 header (totalLen) */
    vfdArray[5].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[5].offset     = vfdArray[4].offset + vfdArray[4].cycleCount;
    vfdArray[5].cycleCount = 2;
    vfdArray[5].patternPtr[0] = 0;
    vfdArray[5].patternPtr[1] = 0x56;
    if(retainCrc == GT_TRUE)
    {
        vfdArray[5].patternPtr[1] += (GT_U8)retainCrcNumBytes;
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    if(testEgressPortAsDsa == GT_TRUE)
    {
        sizeDSATag = (TGF_eDSA_TAG_SIZE_CNS *((testEgressDsaType == TGF_DSA_4_WORD_TYPE_E) ? 1 : 0)) +
                     (TGF_DSA_TAG_SIZE_CNS * ((testEgressDsaType == TGF_DSA_2_WORD_TYPE_E) ? 1 : 0)) -
                     TGF_VLAN_TAG_SIZE_CNS;

        prvTgfEgressPortCascadeSet(&portInterface, GT_TRUE, testEgressDsaType);

        if(testEgressDsaType != TGF_DSA_4_WORD_TYPE_E)
        {
            currentDsaWordsVfdPtr = &EgressCascadePort_DsaWordsVfd[0];
            if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
            {
                /* <SrcTagged> not indicate tag1 , but tag 0 only (tag 1 was recognized at ingress tag 0 not)*/
                currentDsaWordsVfdPtr->patternPtr[0] |= 1 << 5;/*bit 29 word 0*/

                /* those device not support setting the 'default port vid1' and are with value 0 */
                currentDsaWordsVfdPtr->patternPtr[2] &= 0xF0;
                currentDsaWordsVfdPtr->patternPtr[3]  = 0x00;
            }
        }
        else
        {
            currentDsaWordsVfdPtr = &EgressCascadePort_eDsaWordsVfd[0];
        }

       /* update the DSA with runtime info */
        {
            GT_U8   dsaBytes[16];
            GT_U8*  dsaBytesPtr = &dsaBytes[0];
            TGF_PACKET_DSA_TAG_STC tmpDsa;
            GT_U32  ii,jj,kk;
            GT_U32  currentOffset = 0;

            for(jj = 0 ; jj < 2 ; jj++)
            {
                kk = 0;
                for(ii=0; ii< currentDsaWordsVfdPtr[jj].cycleCount ;ii++,kk++,currentOffset++)
                {
                    dsaBytes[currentOffset] = currentDsaWordsVfdPtr[jj].patternPtr[kk];
                }
            }

            for(/*currentOffset continue*/;currentOffset < 16 ; currentOffset++)
            {
                dsaBytes[currentOffset] = 0;
            }

            rc = prvTgfTrafficGeneratorPacketDsaTagParse(dsaBytesPtr,
                &tmpDsa);
            UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagParse: %d",
                                      prvTgfDevNum);

            /* update the DSA tag format */
            tmpDsa.dsaInfo.forward.source.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
            tmpDsa.dsaInfo.forward.origSrcPhy.portNum = tmpDsa.dsaInfo.forward.source.portNum;

            tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
            tmpDsa.dsaInfo.forward.dstEport = tmpDsa.dsaInfo.forward.dstInterface.devPort.portNum;


            /* convert the DSA into into bytes back */
            rc = prvTgfTrafficGeneratorPacketDsaTagBuild(&tmpDsa,dsaBytesPtr);
            UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfTrafficGeneratorPacketDsaTagBuild: %d",
                                      prvTgfDevNum);

            if(testEgressDsaType == TGF_DSA_4_WORD_TYPE_E)
            {
                /* the device want bit 25 in word 3 to be set to 1 <Src Tag0 Is Outer Tag> */
                dsaBytes[(3*4)+0] |= 1 << 1;
                if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                {
                    
                    /* <Skip SA lookup> bit 30 in word 2 always '1' in control learning */
                    dsaBytes[(2*4)+0] |= 1 << 6;
                }
            }

            currentOffset = 0;

            for(jj = 0 ; jj < 2 ; jj++)
            {
                kk = 0;
                for(ii=0; ii< currentDsaWordsVfdPtr[jj].cycleCount ;ii++,kk++,currentOffset++)
                {
                    currentDsaWordsVfdPtr[jj].patternPtr[kk] = dsaBytes[currentOffset];
                }
            }

        }

        vfdArray[1] = currentDsaWordsVfdPtr[0];
        vfdArray[2] = currentDsaWordsVfdPtr[1];
        /* re-count offsets in the vfd array */
        vfdArray[3].offset = vfdArray[2].offset + vfdArray[2].cycleCount;
        vfdArray[4].offset = vfdArray[3].offset + vfdArray[3].cycleCount;
        vfdArray[5].offset = vfdArray[4].offset + vfdArray[4].cycleCount;
    }

    /* AUTODOC: send 3 Ethernet packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:25, VID=5 */
    /* AUTODOC:   EtherType=0x0030 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get 3 tunneled packets on port 2 with: */
    /* AUTODOC:   DA=88:99:77:66:55:88, SA=00:00:00:00:00:05, VID=5 */
    /* AUTODOC:   EtherType=0x6558 */
    /* AUTODOC:   passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:25 */
    /* AUTODOC:   passenger EtherType=0x0030 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {

        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            /* if this is Puma device */

            /* calculate packet length */
            packetSize = prvTgfPacketInfo.totalLen +
                    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS +
                     TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS)  * (portIter == PRV_TGF_EGR_PORT_IDX_CNS);

        }
        else
        {
            /* calculate packet length */
            packetSize = prvTgfPacketInfo.totalLen +
                    (TGF_L2_HEADER_SIZE_CNS +
                     TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS)  * (portIter == PRV_TGF_EGR_PORT_IDX_CNS);


        }

        if(portIter == PRV_TGF_EGR_PORT_IDX_CNS && retainCrc == GT_TRUE)
        {
            packetSize += retainCrcNumBytes;
        }

        if(testEgressPortAsDsa == GT_TRUE && portIter == PRV_TGF_EGR_PORT_IDX_CNS)
        {
            packetSize +=sizeDSATag;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountRxTxArr[portIter],
                                    prvTgfPacketsCountRxTxArr[portIter],
                                    packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                           PRV_TGF_VFD_NUM_CNS, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL1_STRING_MAC((1 << (PRV_TGF_VFD_NUM_CNS * prvTgfBurstCount)) - 1, numTriggers,
                                 "wrong numTriggers =  %d\n", numTriggers);

    for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
    {
        /* check if captured packet has all correct vfd's */
        UTF_VERIFY_EQUAL1_STRING_MAC((1 << PRV_TGF_VFD_NUM_CNS) - 1,
                      numTriggers & ((1 << PRV_TGF_VFD_NUM_CNS) - 1),
            "   Errors while triggers summary result checking: packetIter = %d\n", packetIter);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, numTriggers & BIT_0,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                        prvTgfPacketL2Part.daMac[0],
                                        prvTgfPacketL2Part.daMac[1],
                                        prvTgfPacketL2Part.daMac[2],
                                        prvTgfPacketL2Part.daMac[3],
                                        prvTgfPacketL2Part.daMac[4],
                                        prvTgfPacketL2Part.daMac[5]);

        triggerIndex = 1;
        /* check if captured packet has the same vlan Ethertype */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, ((numTriggers >> triggerIndex) & 1),
                "\n   Ethertype of captured packet must be: %04X",
                                       vfdArray[triggerIndex].patternPtr[0] << 8 | vfdArray[triggerIndex].patternPtr[1]);
        triggerIndex++;

        /* check if captured packet has the same vlan id */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, ((numTriggers >> triggerIndex) & 1),
                "\n   vlan id of captured packet must be: %X",
                                       vfdArray[triggerIndex].patternPtr[0] << 8 | vfdArray[triggerIndex].patternPtr[1]);
        triggerIndex++;

        /* check if captured packet has the same ethertype */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, ((numTriggers >> triggerIndex) & 1),
                "\n   ethertype of captured packet must be: %02X",
                                       vfdArray[triggerIndex].patternPtr[0] << 8 | vfdArray[triggerIndex].patternPtr[1]);
        triggerIndex++;

        /* check if captured packet has the same ipv4 version */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, ((numTriggers >> triggerIndex) & 1),
                "\n   ipv4 version of captured packet must be: %X",
                                       vfdArray[triggerIndex].patternPtr[0] << 8 | vfdArray[triggerIndex].patternPtr[1]);
        triggerIndex++;

        /* check if captured packet has the same ipv4 total length */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, ((numTriggers >> triggerIndex) & 1),
                "\n   ipv4 total length of captured packet must be: %X",
                                       vfdArray[triggerIndex].patternPtr[0] << 8 | vfdArray[triggerIndex].patternPtr[1]);
        triggerIndex++;

        /* prepare next iteration */
        numTriggers = numTriggers >> PRV_TGF_VFD_NUM_CNS;

    }
    /* restore values */
    if(testEgressPortAsDsa == GT_TRUE)
    {
        prvTgfEgressPortCascadeSet(&portInterface, GT_FALSE, testEgressDsaType);
        testEgressPortAsDsa = GT_FALSE;
        testEgressDsaType = TGF_DSA_LAST_TYPE_E;
    }

}

/**
* @internal prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelStartEtherOverIpv4GreRedirectToEgressConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable ingress policy on port 0 */
    rc = prvTgfPclPortIngressPolicyEnable(
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rule 1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_PCL_RULE_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfPclRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* Invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, portsArrayForRestore, sizeof(portsArrayForRestore));

}

