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
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* nextHop VLAN Id  */
static GT_U16 prvTgfNextHopeVlanid = PRV_TGF_NEXTHOPE_VLANID_CNS;

/* if firstCallToLpmConfig= GT_TRUE then save default status
   of bypassing lookup stages 8-31 */
static GT_BOOL       firstCallToLpmConfig=GT_TRUE;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U16         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

static GT_U32        prvTgfFdbRoutingArpIndex  = 3;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x33};
static TGF_MAC_ADDR  prvTgfFdbArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x44};

/* use ePort in the test */
static GT_BOOL       useEPort = GT_FALSE;
/* use ePort mapped to VIDX in the test */
static GT_BOOL       useEPortMappedToVidx = GT_FALSE;

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   (UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)  - 5)

/* VIDX that represents the EPort*/
#define PRV_TGF_VIDX_FROM_EPORT_CNS   (3333 % (PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VIDX_MAC(prvTgfDevNum)+1))


#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

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
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC            egressInfo;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    PRV_TGF_INLIF_PORT_MODE_ENT                 sendPortInlifMode;
    GT_BOOL                                     defaultBypassEnable;
    GT_U32                                      globalMacSaIndex;
    GT_BOOL                                     defaultSourceEportSipLookupEnable;
    GT_BOOL                                     defaultSourceEportSipSaEnable;

    GT_ETHERADDR                                macSaBaseOrig;
    GT_BOOL                                     macSaBaseOrigInitialized;

    CPSS_MAC_SA_LSB_MODE_ENT                    macSaModeOrig;
    GT_U32                                      macSaModeOrigInitialized;

    GT_BOOL                                     macSaModifyEnableOrig;
    GT_U32                                      macSaModifyEnableOrigInitialized;

    PRV_TGF_MAC_HASH_FUNC_MODE_ENT              hashModeOrig;
    GT_BOOL                                     fdbRoutingPortEnableOrig;
    GT_ETHERADDR                                arpMacAddrOrig;
    GT_ETHERADDR                                fdbArpMacAddrOrig;
    CPSS_PACKET_CMD_ENT                         nhPacketCmdOrig;
} prvTgfRestoreCfg;
/* parameters that is needed to be restored */


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpv4UcRoutingPriorityBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] fdbPortNum               - the FDB port num to set in the macEntry destination Interface
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingPriorityBaseConfigurationSet
(
    GT_U32   fdbPortNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {0, 0, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    /* save default VLAN 1 */
    prvTgfBrgVlanEntryStore(1);

    /* NOTE : the vid used also in other files (but without similar logic) as
        'HARD CODED' : 'PRV_TGF_NEXTHOPE_VLANID_CNS' ,
        so prvTgfNextHopeVlanid here must be +4K (or +0) value from 'PRV_TGF_NEXTHOPE_VLANID_CNS'

        meaning that we can not decide to work with any prvTgfNextHopeVlanid value
        for eArch devices !
    */
    prvTgfNextHopeVlanid =
        ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(PRV_TGF_NEXTHOPE_VLANID_CNS + _4K);

    /* update next hop VLAN ID according to eArch */
    prvTgfVlanArray[1] = prvTgfNextHopeVlanid;

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(prvTgfNextHopeVlanid,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
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

    /* AUTODOC: enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
        * Set the FDB entry With DA_ROUTE
        */

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = fdbPortNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceId                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                      sendPortNum         - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] enableIpv4McRouting      - whether to enable IPv4 MC routing in the virtual router
* @param[in] enableIpv6McRouting      - whether to enable IPv6 MC routing in the virtual router
*                                       None
*/
static GT_VOID prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum,
    GT_BOOL  enableIpv4McRouting,
    GT_BOOL  enableIpv6McRouting
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    GT_TRUNK_ID nextHopTrunkId;/* trunk Id for the next hop port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_U32                                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                                  ii;
    GT_U8                                   prvSendPortIndex;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    PRV_TGF_IP_LTT_ENTRY_STC                *defIpv4McLttPtr = NULL;
    PRV_TGF_IP_LTT_ENTRY_STC                *defIpv6McLttPtr = NULL;
    GT_U32                                  numOfPaths;
    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    targetHwDevice = prvTgfDevNum;
    targetPort = nextHopPortNum;

    if (useEPort == GT_TRUE)
    {
        /* save ePort attributes configuration */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.egressInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

        /* set ePort attributes configuration */
        cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &egressInfo);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

        /* save ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.physicalInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

        /* set ePort mapping configuration */
        physicalInfo.type = CPSS_INTERFACE_PORT_E;

        physicalInfo.devPort.hwDevNum = targetHwDevice;
        physicalInfo.devPort.portNum = targetPort;
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &physicalInfo);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
    }
    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
     */

    /* save current port inlif mode */
    rc = prvTgfInlifPortModeGet(sendPortIndex,&(prvTgfRestoreCfg.sendPortInlifMode));
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeGet);
    /* set port inlif mode */
    rc = prvTgfInlifPortModeSet(sendPortIndex,PRV_TGF_INLIF_PORT_MODE_PORT_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);

    cpssOsMemSet(&inlifEntry,0,sizeof(inlifEntry));
    inlifEntry.bridgeEnable                   = GT_TRUE;
    inlifEntry.unkSaUcCommand                 = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipv4UcRouteEnable              = GT_TRUE;
    inlifEntry.vrfId                          = prvUtfVrfId;

    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,sendPortIndex,&inlifEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifEntrySet);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */


    /* Routing from trunk support:
       check if the port is trunk member.
       when trunk member , set all the members with same configuration.
    */
    trunkMember.port = prvTgfPortsArray[sendPortIndex];
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
    if(rc == GT_OK && appDemo_trunk_phy1690_WA_B == 0)
    {
        /* the port is member of the trunk */

        /* get all trunk members */
        numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,senderTrunkId,&numOfEnabledMembers,enabledMembersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvCpssGenericTrunkDbEnabledMembersGet: %d %d",
                                     prvTgfDevNum, senderTrunkId);
        if(numOfEnabledMembers == 0)
        {
            /* empty trunk ??? */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_EMPTY);
            return;
        }

        for(ii = 0 ; ii < numOfEnabledMembers; ii++)
        {
            /* enable Unicast IPv4 Routing on the trunk members */

            rc = prvTgfDefPortsArrayPortToPortIndex(enabledMembersArray[ii].port,&prvSendPortIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArrayPortToPortIndex: %d", prvSendPortIndex);

            rc = prvTgfIpPortRoutingEnable(prvSendPortIndex, CPSS_IP_UNICAST_E,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                         enabledMembersArray[ii].hwDevice, enabledMembersArray[ii].port);
        }
    }
    else
    if(rc == GT_OK && appDemo_trunk_phy1690_WA_B)
    {
        GT_PORT_NUM eportOfTrunk;

        rc = trunkEPortGet_phy1690_WA_B(senderTrunkId,&eportOfTrunk);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d senderTrunkId[%d]",
                                     prvTgfDevNum, senderTrunkId);

        /* in this WA we get srcEPort that represents the trunkId ..
           so need to enable it's routing */
#ifdef CHX_FAMILY
    /* call device specific API */
        rc = cpssDxChIpPortRoutingEnable(prvTgfDevNum, eportOfTrunk, CPSS_IP_UNICAST_E,
                                           CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
#endif /* CHX_FAMILY */
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChIpPortRoutingEnable: prvTgfDevNum %d eportOfTrunk %d",
                                     prvTgfDevNum, eportOfTrunk);
    }
    else
    {
        /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
        rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, sendPortIndex);
    }

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

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
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = prvTgfNextHopeVlanid;

    if(useEPortMappedToVidx == GT_FALSE)
    {
        /* Routing to trunk support:
           check if the NH port is trunk member.
           when trunk member --> use the trunk ID.
        */
        trunkMember.port = targetPort;
        trunkMember.hwDevice = targetHwDevice;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
        if(rc == GT_OK && appDemo_trunk_phy1690_WA_B == 0)
        {
            /* the port is member of the trunk */
            regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_TRUNK_E;
            regularEntryPtr->nextHopInterface.trunkId = nextHopTrunkId;
        }
        else
        if(rc == GT_OK && appDemo_trunk_phy1690_WA_B)
        {
            GT_PORT_NUM eportOfTrunk;

            rc = trunkEPortGet_phy1690_WA_B(nextHopTrunkId,&eportOfTrunk);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d nextHopTrunkId[%d]",
                                         prvTgfDevNum, nextHopTrunkId);

            regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
            regularEntryPtr->nextHopInterface.devPort.hwDevNum = targetHwDevice;
            regularEntryPtr->nextHopInterface.devPort.portNum = eportOfTrunk;
        }
        else
        {
            regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
            regularEntryPtr->nextHopInterface.devPort.hwDevNum = targetHwDevice;

            if (useEPort == GT_TRUE)
                regularEntryPtr->nextHopInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;
            else
                regularEntryPtr->nextHopInterface.devPort.portNum = targetPort;
        }
    }
    else /* useEPortMappedToVidx == GT_TRUE */
    {
        /* Routing to EPort that represents VIDX from the E2PHY */
        regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
        regularEntryPtr->nextHopInterface.devPort.hwDevNum = targetHwDevice;
        regularEntryPtr->nextHopInterface.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;
    }
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0,
                         sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* define max number of paths */
        if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            numOfPaths = 0;
        }
        else
        {
            numOfPaths = 1;
        }

        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;
            }
            if (ii == 1)
            {
                ipLttEntryPtr = &defIpv6UcRouteEntryInfo.ipLttEntry;
            }
            /* set defUcRouteEntryInfo */
            ipLttEntryPtr->routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipLttEntryPtr->numOfPaths               = numOfPaths;
            ipLttEntryPtr->routeEntryBaseIndex      = prvTgfRouteEntryBaseIndex;
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

            if ((ii == 0) && (enableIpv4McRouting == GT_TRUE))
            {
                defIpv4McLttPtr = ipLttEntryPtr;
            }
            if ((ii == 1) && (enableIpv6McRouting == GT_TRUE))
            {
                defIpv6McLttPtr = ipLttEntryPtr;
            }
        }

        /* disable Patricia trie validity */
        prvTgfIpValidityCheckEnable(GT_FALSE);

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         defIpv4McLttPtr, defIpv6McLttPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.priority  = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E;

    /* AUTODOC: for regular test add IPv4 UC prefix 1.1.1.3/32 */

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}



/**
* @internal prvTgfIpv4UcRoutingPriorityBasicRoutingConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useBulk                  - whether to use bulk operation to add prefixes
* @param[in] enableIpv4McRouting      - whether to enable IPv4 MC routing in the virtual router,
*                                      not relevant for policy based routing.
* @param[in] enableIpv6McRouting      - whether to enable IPv6 MC routing in the virtual router,
*                                      not relevant for policy based routing.
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingPriorityBasicRoutingConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;
    GT_BOOL  enableIpv4McRouting = GT_FALSE;
    GT_BOOL  enableIpv6McRouting = GT_FALSE;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            prvTgfBasicIpv4UcRoutingLttRouteConfigurationSet(prvUtfVrfId, sendPortIndex,
                                                             nextHopPortNum, enableIpv4McRouting,
                                                             enableIpv6McRouting);
            break;
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:
            break;
    }
}

/**
* @internal prvTgfIpv4UcRoutingPriorityTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] expectFdbRouting         - expect FDB routing if value is GT_TRUE 
*
*/
GT_VOID prvTgfIpv4UcRoutingPriorityTrafficGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
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
    GT_U32       egressPortIndex = 0xFF;
    GT_BOOL      isRoutedPacket = GT_FALSE;
    GT_BOOL      is_prvTgfCaptureForceTtiDisableModeSet_GT_TRUE = GT_FALSE;

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

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

    if (expectFdbRouting)
        cpssOsMemCpy(arpMacAddr, &prvTgfFdbArpMac, sizeof(TGF_MAC_ADDR));
    else
        cpssOsMemCpy(arpMacAddr, &prvTgfArpMac, sizeof(TGF_MAC_ADDR));

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
}

/**
* @internal prvTgfIpv4UcRoutingPrioritySetLeafPriority function
* @endinternal
*
* @brief   Set routing priority between LPM and FDB loopup
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] priority                 - priority
*
*/
GT_VOID prvTgfIpv4UcRoutingPrioritySetLeafPriority
(
    GT_U32 prvUtfVrfId,
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority
)
{
    GT_STATUS                               rc;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.priority  = priority;



    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpv4UcRoutingPriorityConfigurationRestore function
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
GT_VOID prvTgfIpv4UcRoutingPriorityConfigurationRestore
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
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_TRUNK_ID senderTrunkId;/* trunk Id for the sender port that is member of */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/* temporary trunk member */
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  ii;
    GT_U8                   prvSendPortIndex;
    CPSS_PORTS_BMP_STC      portBitmap;


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

    if (useEPort == GT_TRUE)
    {
        /* restore ePort attributes configuration */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.egressInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

        /* restore ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.physicalInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
    }

    if(useEPortMappedToVidx == GT_TRUE)
    {
        useEPortMappedToVidx = GT_FALSE;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
        /* AUTODOC: delete the VIDX  */
        rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_VIDX_FROM_EPORT_CNS, &portBitmap);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d",
                                     prvTgfDevNum);

        /* restore ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               PRV_TGF_DESTINATION_EPORT_CNS,
                                                               &(prvTgfRestoreCfg.physicalInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
    }

    /* -------------------------------------------------------------------------
        * 2. Restore Route Configuration
        */
    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);


    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {

        /* Routing from trunk support:
            check if the port is trunk member.
            when trunk member , set all the members with same configuration.
        */
        trunkMember.port = prvTgfPortsArray[sendPortIndex];
        trunkMember.hwDevice = prvTgfDevNum;

        rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&senderTrunkId);
        if(rc == GT_OK)
        {
            /* the port is member of the trunk */

            /* get all trunk members */
            numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
            rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,senderTrunkId,&numOfEnabledMembers,enabledMembersArray);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvCpssGenericTrunkDbEnabledMembersGet: %d %d",
                                            prvTgfDevNum, senderTrunkId);
            if(numOfEnabledMembers == 0)
            {
                /* empty trunk ??? */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_EMPTY);
                return;
            }

            for(ii = 0 ; ii < numOfEnabledMembers; ii++)
            {
                /* enable Unicast IPv4 Routing on the trunk members */
                rc = prvTgfDefPortsArrayPortToPortIndex(enabledMembersArray[ii].port,&prvSendPortIndex);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefPortsArrayPortToPortIndex: %d", prvSendPortIndex);

                rc = prvTgfIpPortRoutingEnable(prvSendPortIndex, CPSS_IP_UNICAST_E,
                                                CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                                enabledMembersArray[ii].hwDevice, enabledMembersArray[ii].port);
            }
        }
        else
        {
            /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
            rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,
                                            CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                            prvTgfDevNum, sendPortIndex);
        }
    }

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* restore port inlif mode */
    rc = prvTgfInlifPortModeSet(sendPortIndex,prvTgfRestoreCfg.sendPortInlifMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);

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
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &prvTgfRestoreCfg.arpMacAddrOrig);
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

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* For SIP5 devices LPM Bypass is supported */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
       if(firstCallToLpmConfig==GT_FALSE)
       {
           rc = prvTgfLpmLastLookupStagesBypassEnableSet(prvTgfDevNum,prvTgfRestoreCfg.defaultBypassEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLastLookupStagesBypassEnableSet: %d", prvTgfDevNum);

           rc = prvTgfLpmPortSipLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[sendPortIndex], prvTgfRestoreCfg.defaultSourceEportSipLookupEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChLpmPortSipLookupEnableSet: %d", prvTgfDevNum);

           rc = prvTgfIpPortSipSaEnableSet(prvTgfDevNum,prvTgfPortsArray[sendPortIndex], prvTgfRestoreCfg.defaultSourceEportSipSaEnable);
           UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpPortSipSaEnableSet: %d", prvTgfDevNum);

           firstCallToLpmConfig=GT_TRUE;
       }
    }

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore index to global MAC SA table in eport attribute table */
        rc =  prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], prvTgfRestoreCfg.globalMacSaIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");
    }

    if(prvTgfRestoreCfg.macSaModeOrigInitialized)
    {
        /* AUTODOC: restore MAC SA LSB mode for port  */
        rc = prvTgfIpPortRouterMacSaLsbModeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], prvTgfRestoreCfg.macSaModeOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpPortRouterMacSaLsbModeSet");
        prvTgfRestoreCfg.macSaModeOrigInitialized = 0;
    }

    if(prvTgfRestoreCfg.macSaModifyEnableOrigInitialized)
    {
        /* AUTODOC: restore MAC SA modify enable  */
        rc = prvTgfIpRouterMacSaModifyEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], prvTgfRestoreCfg.macSaModifyEnableOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaModifyEnable");
        prvTgfRestoreCfg.macSaModifyEnableOrigInitialized = 0;
    }

    if(prvTgfRestoreCfg.macSaBaseOrigInitialized)
    {
        /* AUTODOC: restore MAC SA Base for router  */
        rc = prvTgfIpRouterMacSaBaseSet(prvTgfDevNum, &prvTgfRestoreCfg.macSaBaseOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseSet");
        prvTgfRestoreCfg.macSaBaseOrigInitialized = 0;
    }

    /* restore default VLAN 1 */
    prvTgfBrgVlanEntryRestore(1);
}

/**
* @internal prvTgfIpv4UcRoutingPriorityFdbRoutingGenericConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfIpv4UcRoutingPriorityFdbRoutingGenericConfigurationSet
(
    GT_U8   sendPortIndex
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: get current  CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeGet(prvTgfDevNum, &prvTgfRestoreCfg.hashModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeGet");

    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* AUTODOC: Get enable/disable status of  using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableGet( prvTgfDevNum, prvTgfPortsArray[sendPortIndex],
                                           CPSS_IP_PROTOCOL_IPV4_E, &prvTgfRestoreCfg.fdbRoutingPortEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableGet");

    /* AUTODOC: Enable using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[sendPortIndex],
                                          CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}


/**
* @internal prvTgfIpv4UcRoutingPriorityFdbRoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcRoutingPriorityFdbRoutingConfigurationSet
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
#if 1
    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
#endif
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
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_NEXTHOPE_VLANID_CNS;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfFdbRoutingArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    prvTgfFdbIpv4UcRoutingRouteConfigurationSet(GT_TRUE, &macEntry);
}