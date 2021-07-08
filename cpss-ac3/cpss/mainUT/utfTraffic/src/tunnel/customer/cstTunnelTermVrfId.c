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
* @file cstTunnelTermVrfId.c
*
* @brief CST: Tunnel Term: L3 Interface based on (port, vlan)
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>

#ifdef CHX_FAMILY
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>


#include <common/customer/cstCommon.h>
#include <common/customer/cstBridge.h>
#include <common/customer/cstIp.h>

#include <tunnel/prvTgfTunnelStartIpv4overGreIpv4.h>
#include <tunnel/customer/cstTunnelTermVrfId.h>


extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* GRE header size (in bytes) */
#define CST_GRE_HEADER_SIZE_CNS           4

/* default VLAN Id */
#define PRV_CST_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_CST_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_CST_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_CST_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_CST_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_CST_NEXTHOPE_PORT_IDX_CNS     3

/* source ePort number, assigned by TTI */
#define PRV_CST_SRC_EPORT_CNS             22

/* the counter set for a route entry is linked to */
#define PRV_CST_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* the Route entry index for UC Route entry Table */
static GT_U32        prvCstRouteEntryBaseIndex = 5;

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32        prvCstRouterArpTunnelStartLineIndex = 8;

/* the LPM DB id for LPM Table */
static GT_U32        prvCstLpmDBId             = 0;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvCstTunnelMacDa = {0x88, 0x77, 0x11, 0x11, 0x55, 0x66};

/* Tunnel destination IP */
static TGF_IPV4_ADDR prvCstTunnelDstIp = {1, 1, 1, 2};

/* Tunnel source IP */
static TGF_IPV4_ADDR prvCstTunnelSrcIp = {2, 2, 3, 3};

/* Test device number */
static GT_U8    swDevNum = 0;

static GT_U32   errorLine = 0;

/* used ports */
static  GT_U32    portsArray[] = {0, 18, 36, 58};

/* vrfId */
static  GT_U32   prvCstVrfId = 0;

/* struct to save info for config restore */
static struct{

    CPSS_DXCH_TTI_MAC_VLAN_STC dxchValue;
    CPSS_DXCH_TTI_MAC_VLAN_STC dxchMask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  dxchInterfaceInfo;

    GT_BOOL  ttiEnable;

}orig;

/* tti rule index */
static GT_U32  ruleIndex = 0;

/* mac2me index */
static GT_U32  mac2meIndex = 0;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvCstPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvCstPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x73C9,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  2,  1,  3}    /* dstAddr */
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal cstTunnelTermVrfIdSetMacToMe function
* @endinternal
*
* @brief   Set mac to me for (port, mac, vlan)
*/
static GT_STATUS cstTunnelTermVrfIdSetMacToMe
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;

    CPSS_DXCH_TTI_MAC_VLAN_STC dxchValue;
    CPSS_DXCH_TTI_MAC_VLAN_STC dxchMask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  dxchInterfaceInfo;


    cpssOsMemSet(&dxchValue, 0, sizeof(dxchValue));
    cpssOsMemSet(&dxchMask, 0, sizeof(dxchMask));

    /* set mac */
    cpssOsMemCpy((GT_VOID*)dxchValue.mac.arEther, (GT_VOID*)prvCstPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet((GT_VOID*)dxchMask.mac.arEther, 0xFF, sizeof(dxchMask.mac.arEther));

    /* set vid */
    dxchValue.vlanId = PRV_CST_SEND_VLANID_CNS;
    dxchMask.vlanId  = 0xFFF;

    /* set src port */
    dxchInterfaceInfo.includeSrcInterface = CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E;
    dxchInterfaceInfo.srcIsTrunk          = GT_FALSE;
    dxchInterfaceInfo.srcPortTrunk        = portsArray[PRV_CST_SEND_PORT_IDX_CNS];

    rc = cpssDxChTtiMacToMeGet(swDevNum, mac2meIndex, &orig.dxchValue, &orig.dxchMask, &orig.dxchInterfaceInfo);
    CHECK_RC_MAC(rc);

    rc = cpssDxChTtiMacToMeSet(swDevNum, mac2meIndex, &dxchValue, &dxchMask, &dxchInterfaceInfo);
    CHECK_RC_MAC(rc);

    return rc;
}

/**
* @internal cstTunnelTermVrfIdSetTti function
* @endinternal
*
* @brief   Set TTI rule: TTI rule classifies (port, VLAN)
*         and Action assigns VRF-ID, Src ePort, eVLAN
*/
static GT_STATUS cstTunnelTermVrfIdSetTti
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;

    CPSS_DXCH_TTI_RULE_UNT   dxChPattern;
    CPSS_DXCH_TTI_RULE_UNT   dxChMask;
    CPSS_DXCH_TTI_ACTION_UNT dxChTtiAction;

    cpssOsMemSet((GT_VOID*) &dxChPattern,   0, sizeof(dxChPattern));
    cpssOsMemSet((GT_VOID*) &dxChMask,      0, sizeof(dxChMask));
    cpssOsMemSet((GT_VOID*) &dxChTtiAction, 0, sizeof(dxChTtiAction));

    ruleIndex = appDemoDxChTcamTtiBaseIndexGet(swDevNum, 0);

    /* AUTODOC: enable the TTI lookup for IPv4 at the port 0 */
    rc = cpssDxChTtiPortLookupEnableGet(swDevNum, portsArray[PRV_CST_SEND_PORT_IDX_CNS],
                                        CPSS_DXCH_TTI_KEY_IPV4_E, &orig.ttiEnable);
    CHECK_RC_MAC(rc);

    rc = cpssDxChTtiPortLookupEnableSet(swDevNum, portsArray[PRV_CST_SEND_PORT_IDX_CNS],
                                        CPSS_DXCH_TTI_KEY_IPV4_E, GT_TRUE);
    CHECK_RC_MAC(rc);


    /* AUTODOC: configure tti rule */
    dxChPattern.ipv4.common.vid           = PRV_CST_SEND_VLANID_CNS;
    dxChMask.ipv4.common.vid              = 0xFFF;

    dxChPattern.ipv4.common.srcPortTrunk  = portsArray[PRV_CST_SEND_PORT_IDX_CNS];
    dxChMask.ipv4.common.srcPortTrunk     = 0xff;

    dxChTtiAction.type2.command                     = CPSS_PACKET_CMD_FORWARD_E;
    dxChTtiAction.type2.vrfId                       = prvCstVrfId;
    dxChTtiAction.type2.sourceEPortAssignmentEnable = GT_TRUE;
    dxChTtiAction.type2.sourceEPort                 = PRV_CST_SRC_EPORT_CNS;
    dxChTtiAction.type2.tag0VlanId                  = PRV_CST_NEXTHOPE_VLANID_CNS;
    dxChTtiAction.type2.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
    dxChTtiAction.type2.tag1VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E; /* must be set */

    /* AUTODOC: set tti rule */
    rc = cpssDxChTtiRuleSet(swDevNum, ruleIndex,
                                CPSS_DXCH_TTI_KEY_IPV4_E,
                                &dxChPattern, &dxChMask,
                                CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
                                &dxChTtiAction);

    CHECK_RC_MAC(rc);

    return rc;
}

/**
* @internal cstTunnelTermVrfIdBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
static GT_STATUS cstTunnelTermVrfIdBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;

    /* SETUP CONFIGURATION: */
    cpssOsPrintf("======= Setting Base Configuration =======\n");

    /* create VLAN 5 with untagged ports [0,1] */
    /* create VLAN 6 with untagged ports [2,3] */

    rc = cstBrgDefVlanEntryWithPortsSet(swDevNum,PRV_CST_SEND_VLANID_CNS,
            portsArray, NULL, NULL, 2);
    CHECK_RC_MAC(rc);

    rc = cstBrgDefVlanEntryWithPortsSet(swDevNum, PRV_CST_NEXTHOPE_VLANID_CNS,
            portsArray + 2, NULL, NULL, 2);
    CHECK_RC_MAC(rc);


    /* set mac to me for {port, mac, vlan}*/
    rc = cstTunnelTermVrfIdSetMacToMe();
    CHECK_RC_MAC(rc);

    /* set TTI rule: TTI rule classifies (port, VLAN) and Action assigns VRF-ID, Src ePort, eVLAN*/
    rc = cstTunnelTermVrfIdSetTti();
    CHECK_RC_MAC(rc);


    {
        CPSS_MAC_ENTRY_EXT_STC   macEntry;
        GT_HW_DEV_NUM            targetHwDevice;

        /* enable VLAN based MAC learning for VLAN 6 */
        rc = cpssDxChBrgVlanLearningStateSet(swDevNum, PRV_CST_NEXTHOPE_VLANID_CNS, GT_TRUE);
        CHECK_RC_MAC(rc);

        rc = cpssDxChCfgHwDevNumGet((GT_U8)swDevNum, &targetHwDevice);
        CHECK_RC_MAC(rc);

        /* create a macEntry with .daRoute = GT_TRUE */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvCstPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId         = PRV_CST_NEXTHOPE_VLANID_CNS;
        macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum  = targetHwDevice;
        macEntry.dstInterface.devPort.portNum   = portsArray[PRV_CST_FDB_PORT_IDX_CNS];
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
        rc = cpssDxChBrgFdbMacEntrySet(swDevNum, &macEntry);
        CHECK_RC_MAC(rc);
    }

    return rc;
}

/**
* @internal cstTunnelTermVrfIdLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
static GT_STATUS cstTunnelTermVrfIdLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC         routeEntriesArray[1];
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC         *regularEntryPtr;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_HW_DEV_NUM                           targetHwDevice;


    cpssOsPrintf("======= Setting Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* enable Unicast IPv4 Routing on port 0 */
    rc = cpssDxChIpPortRoutingEnable(swDevNum, PRV_CST_SRC_EPORT_CNS,
                CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    CHECK_RC_MAC(rc);

    /* enable IPv4 Unicast Routing on Vlan 5 */
    rc = cpssDxChBrgVlanIpUcRouteEnable(swDevNum, PRV_CST_NEXTHOPE_VLANID_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    CHECK_RC_MAC(rc);


    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

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
    regularEntryPtr->entry.regularEntry.countSet                   = PRV_CST_COUNT_SET_CNS;
    regularEntryPtr->entry.regularEntry.trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->entry.regularEntry.sipAccessLevel             = 0;
    regularEntryPtr->entry.regularEntry.dipAccessLevel             = 0;
    regularEntryPtr->entry.regularEntry.ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->entry.regularEntry.scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->entry.regularEntry.siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->entry.regularEntry.mtuProfileIndex            = 0;
    regularEntryPtr->entry.regularEntry.isTunnelStart              = GT_TRUE;
    regularEntryPtr->entry.regularEntry.nextHopVlanId              = PRV_CST_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->entry.regularEntry.nextHopInterface.type      = CPSS_INTERFACE_PORT_E;

    regularEntryPtr->entry.regularEntry.nextHopInterface.devPort.portNum = portsArray[PRV_CST_NEXTHOPE_PORT_IDX_CNS];
    regularEntryPtr->entry.regularEntry.nextHopARPPointer          = 0;
    regularEntryPtr->entry.regularEntry.nextHopTunnelPointer       = prvCstRouterArpTunnelStartLineIndex;

    rc = cpssDxChCfgHwDevNumGet((GT_U8)swDevNum, &targetHwDevice);
    CHECK_RC_MAC(rc);

    regularEntryPtr->entry.regularEntry.nextHopInterface.devPort.hwDevNum = targetHwDevice;


    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = cpssDxChIpUcRouteEntriesWrite(swDevNum, prvCstRouteEntryBaseIndex, routeEntriesArray, 1);
    CHECK_RC_MAC(rc);


    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvCstRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvCstPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* add IPv4 UC prefix 1.1.1.3/32 */
    rc = cpssDxChIpLpmIpv4UcPrefixAdd(prvCstLpmDBId, prvCstVrfId, &ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    CHECK_RC_MAC(rc);

    return rc;
}

/**
* @internal cstTunnelTermVrfIdTunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
static GT_STATUS cstTunnelTermVrfIdTunnelConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                         rc = GT_OK;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT tunnelEntry;
    CPSS_TUNNEL_TYPE_ENT              tunnelType;


    cpssOsPrintf("======= Setting Tunnel Configuration =======\n");

    cpssOsMemSet(&tunnelEntry,0,sizeof(tunnelEntry));

    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId           = PRV_CST_NEXTHOPE_VLANID_CNS;
    tunnelEntry.ipv4Cfg.ttl              = 33;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvCstTunnelMacDa, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvCstTunnelDstIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvCstTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

    /* add TS entry 8 with: */
    /*   tunnelType=X_OVER_GRE_IPV4 */
    /*   vlanId=6, ttl=33, ethType=TUNNEL_GRE0_ETHER_TYPE */
    /*   DA=88:77:11:11:55:66, srcIp=2.2.3.3, dstIp=1.1.1.2 */

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    }
    else
    {
        tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
    }

    rc = cpssDxChTunnelStartEntrySet(swDevNum, prvCstRouterArpTunnelStartLineIndex,
        tunnelType, &tunnelEntry);
    CHECK_RC_MAC(rc);

    return rc;
}
#endif  /* CHX_FAMILY */

/**
* @internal cstTunnelTermVrfIdConfigSet function
* @endinternal
*
* @brief   Set configuration
*/
GT_STATUS cstTunnelTermVrfIdConfigSet(GT_VOID)
{

    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY
    /* to avoid compilation warnings.
       errorLine is used in CHECK_RC_MAC */
    swDevNum = prvTgfDevNum;
    TGF_PARAM_NOT_USED(errorLine);

    {/* use tested ports , because {0, 18, 36, 58} are not always valid */
        GT_U32  ii;
        for(ii = 0; ii < 4 ; ii++)
        {
            portsArray[ii] = prvTgfPortsArray[ii];
        }
    }


    cpssOsPrintf("\n\n======= Set Demo Configuration =======\n\n\n");

    /* Set Base configuration */
    rc = cstTunnelTermVrfIdBaseConfigurationSet();
    CHECK_RC_MAC(rc);

    /* Set Route configuration */
    rc = cstTunnelTermVrfIdLttRouteConfigurationSet();
    CHECK_RC_MAC(rc);

    /* Set Tunnel configuration */
    rc = cstTunnelTermVrfIdTunnelConfigurationSet();
    CHECK_RC_MAC(rc);
#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;
}

/**
* @internal cstTunnelTermVrfIdConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_STATUS cstTunnelTermVrfIdConfigRestore(GT_VOID)
{

    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY

    cpssOsPrintf("\n\n======= Restore Configuration =======\n\n\n");

    /* Restore configuration */
    prvTgfTunnelStartIpv4overGreIpv4ConfigurationRestore(prvCstVrfId);
    prvTgfPclRestore();

    rc = cpssDxChTtiMacToMeSet(swDevNum, mac2meIndex, &orig.dxchValue, &orig.dxchMask,
                                                    &orig.dxchInterfaceInfo);
    CHECK_RC_MAC(rc);

    rc = cpssDxChTtiPortLookupEnableSet(swDevNum, portsArray[PRV_CST_SEND_PORT_IDX_CNS],
                                        CPSS_DXCH_TTI_KEY_IPV4_E, orig.ttiEnable);
    CHECK_RC_MAC(rc);

    rc = cpssDxChTtiRuleValidStatusSet(swDevNum, ruleIndex, GT_FALSE);
    CHECK_RC_MAC(rc);
#else
    rc = GT_NOT_IMPLEMENTED;
#endif  /* CHX_FAMILY */

    return rc;
}


