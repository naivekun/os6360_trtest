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
* @file prvTgfMpls.c
*
* @brief MPLS Functionality
*
* @version   7
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
#include <common/tgfMplsGen.h>
#include <mpls/prvTgfMpls.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS              1

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           2

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       2

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to in default VLAN */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number of unused port in default VLAN */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* port number to send traffic to */
#define PRV_TGF_FREE_PORT_IDX_CNS         2

/* port number to receive traffic from in nextHop VLAN */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS      3

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/* the Arp Address index of the Router ARP Table
   to write to the NHLFE entry ArpPtr field */
static GT_U32        prvTgfNhlfeArpLineIndex1      = 0;
static GT_U32        prvTgfNhlfeArpLineIndex2      = 1;

/* the NHLFE base index to set in the
Next Hop Label Forwarding Entry and in the MPLS Interface table  */
static GT_U32 nhlfEntryBaseIndex = 16;


/* the ARP MAC address to write to the ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac1 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x11};
static TGF_MAC_ADDR  prvTgfArpMac2 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};


/************************ common parts of packets **************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                 /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/**************************** packet MPLS **********************************/

/* MPLS ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsMplsEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/* MPLS part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMplsPart = {
    0x10,               /* label */
    0,                  /* exp */
    1,                  /* stack */
    0x40                /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfPacketMplsPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMplsPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* MPLS part  - push test*/
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMplsPart2 = {
    0x11,               /* label */
    0,                  /* exp */
    1,                  /* stack */
    0x40                /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfPacketMplsPartArray2[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMplsPart2},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* MPLS part - php test*/
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMplsPart3 = {
    0xA,                /* label */
    0,                  /* exp */
    1,                  /* stack */
    0x40                /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfPacketMplsPartArray3[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMplsPart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMplsPart3},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default inlifMode */
static PRV_TGF_INLIF_PORT_MODE_ENT     inlifMode;

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfMplsBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfMplsBaseConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                     isTagged  = GT_TRUE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_TRUE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;

    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* AUTODOC: create VLAN 2 with untagged ports [0,1,2,3] */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* add ports to vlan member */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanPerPortArray[portIter],
                                     prvTgfPortsArray[portIter], isTagged);
    }

   /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: set PVID 2 for ports [0,1,2,3] */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 2 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfMplsRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfMplsRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;

    PRV_UTF_LOG0_MAC("======= Setting MPLS Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Set Inlif Entry - that needs for Puma devices
     */

    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_FALSE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_FALSE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_FALSE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_TRUE;
    inlifEntry.vrfId             = 1;

    /* SCT Section */
    inlifEntry.ipv4IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.ipv6IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_TRUE;
    inlifEntry.ipSecurityProfile              = 0;
    inlifEntry.ipv4IgmpToCpuEnable            = GT_FALSE;
    inlifEntry.ipv6IcmpToCpuEnable            = GT_FALSE;
    inlifEntry.udpBcRelayEnable               = GT_FALSE;
    inlifEntry.arpBcToCpuEnable               = GT_FALSE;
    inlifEntry.arpBcToMeEnable                = GT_FALSE;
    inlifEntry.ripv1MirrorEnable              = GT_FALSE;
    inlifEntry.ipv4LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6NeighborSolicitationEnable = GT_FALSE;

    /* Misc Section */
    inlifEntry.mirrorToAnalyzerEnable = GT_FALSE;
    inlifEntry.mirrorToCpuEnable      = GT_FALSE;

     /* save inlif port mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_SEND_PORT_IDX_CNS, &inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeGet");

    /* set inlif mode to support single input logical interface for port 0  */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,
                                   PRV_TGF_INLIF_PORT_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeSet");

    /* call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             &inlifEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfInlifEntrySet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfMplsConfigurationSet function
* @endinternal
*
* @brief   Set MPLS Configuration
*/
GT_VOID prvTgfMplsConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT   mode;
    GT_BOOL                             enable;
    GT_U32                              baseIndex;
    PRV_TGF_MPLS_INTERFACE_ENTRY_STC    mplsInterfaceEntryStc;
    PRV_TGF_MPLS_INTERFACE_ENTRY_STC    mplsInterfaceEntryStcGet;
    GT_ETHERADDR                        arpMacAddr;
    CPSS_PACKET_CMD_ENT                 cmd;
    GT_U32                              portIter = 0;

    /* AUTODOC: SETUP MPLS CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting MPLS Configuration =======\n");

    /* AUTODOC: enable MPLS LSR(Label Switch Router) switching */
    rc = prvTgfMplsLsrEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsLsrEnableSet: %d", prvTgfDevNum);

    rc = prvTgfMplsLsrEnableGet(prvTgfDevNum, &enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsLsrEnableGet: %d", prvTgfDevNum);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, enable,"prvTgfMplsLsrEnableGet %d", enable);

    /* AUTODOC: enable MPLS Switching on all interfaces */
    for (portIter = 0; portIter < (GT_U32)(prvTgfPortsNum-1); portIter++)
    {
        rc = prvTgfInlifMplsRouteEnableSet(prvTgfDevNum,PRV_TGF_INLIF_TYPE_PORT_E,
                                           prvTgfPortsArray[portIter],GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"prvTgfInlifMplsRouteEnableSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: define interface Label Space Mode */
    rc = prvTgfMplsLabelSpaceModeSet(prvTgfDevNum, PRV_TGF_MPLS_PER_INTERFACE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsLabelSpaceModeSet: %d", prvTgfDevNum);

    rc = prvTgfMplsLabelSpaceModeGet(prvTgfDevNum, &mode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsLabelSpaceModeGet: %d", prvTgfDevNum);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_MPLS_PER_INTERFACE_E, mode,
                                 "prvTgfMplsLabelSpaceModeGet %d", mode);

    /* AUTODOC: set MPLS Special Labels table base index 0 */
    rc = prvTgfMplsNhlfeSpecialLabelsBaseIndexSet(prvTgfDevNum, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfSpecialLabelsBaseIndexSet: %d", prvTgfDevNum);

    rc = prvTgfMplsNhlfeSpecialLabelsBaseIndexGet(prvTgfDevNum, &baseIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfSpecialLabelsBaseIndexGet: %d", prvTgfDevNum);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(0, baseIndex,
                                 "prvTgfMplsNhlfSpecialLabelsBaseIndexGet: %d", baseIndex);


    /* Create a new entry in the MPLS Interface table */
    mplsInterfaceEntryStc.valid       = GT_TRUE;
    mplsInterfaceEntryStc.minLabel    = 16;
    mplsInterfaceEntryStc.maxLabel    = 26;
    mplsInterfaceEntryStc.baseIndex   = nhlfEntryBaseIndex; /* 16 */
    mplsInterfaceEntryStc.ecmpQosSize = 0;
    mplsInterfaceEntryStc.nextHopRouteMethod = PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_REGULAR_E;

    /* AUTODOC: set MPLS interface table entry 1: */
    /* AUTODOC:   minLabel=16, maxLabel=26 */
    /* AUTODOC:   baseIndex=16, ecmpQosSize=0 */
    /* AUTODOC:   nextHopRouteMethod=REGULAR */
    rc = prvTgfMplsInterfaceEntrySet(prvTgfDevNum, 1, &mplsInterfaceEntryStc);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsInterfaceEntrySet: %d", prvTgfDevNum);

    rc = prvTgfMplsInterfaceEntryGet(prvTgfDevNum, 1, &mplsInterfaceEntryStcGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsInterfaceEntryGet: %d", prvTgfDevNum);

    /* Check fields */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_PUMA_E)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(mplsInterfaceEntryStc.valid, mplsInterfaceEntryStcGet.valid,
                                 "prvTgfMplsInterfaceEntryGet:%d", mplsInterfaceEntryStcGet.valid);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(mplsInterfaceEntryStc.minLabel, mplsInterfaceEntryStcGet.minLabel,
                                 "prvTgfMplsInterfaceEntryGet:%d", mplsInterfaceEntryStcGet.minLabel);

    UTF_VERIFY_EQUAL1_STRING_MAC(mplsInterfaceEntryStc.maxLabel, mplsInterfaceEntryStcGet.maxLabel,
                                 "prvTgfMplsInterfaceEntryGet:%d", mplsInterfaceEntryStcGet.maxLabel);

    UTF_VERIFY_EQUAL1_STRING_MAC(mplsInterfaceEntryStc.baseIndex, mplsInterfaceEntryStcGet.baseIndex,
                                 "prvTgfMplsInterfaceEntryGet:%d", mplsInterfaceEntryStcGet.baseIndex);

    UTF_VERIFY_EQUAL1_STRING_MAC(mplsInterfaceEntryStc.ecmpQosSize, mplsInterfaceEntryStcGet.ecmpQosSize,
                                 "prvTgfMplsInterfaceEntryGet:%d", mplsInterfaceEntryStcGet.ecmpQosSize);

    UTF_VERIFY_EQUAL1_STRING_MAC(mplsInterfaceEntryStc.nextHopRouteMethod, mplsInterfaceEntryStcGet.nextHopRouteMethod,
                                 "prvTgfMplsInterfaceEntryGet:%d", mplsInterfaceEntryStcGet.nextHopRouteMethod);

    /* AUTODOC: set invalid MPLS entry command to TRAP_TO_CPU */
    rc = prvTgfMplsInvalidEntryCmdSet(prvTgfDevNum, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsInvalidEntryCmdSet: %d", prvTgfDevNum);

    rc = prvTgfMplsInvalidEntryCmdGet(prvTgfDevNum, &cmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsInvalidEntryCmdGet: %d", prvTgfDevNum);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_PACKET_CMD_TRAP_TO_CPU_E, cmd,"prvTgfMplsLsrEnableGet %d", cmd);


    /* AUTODOC: write a ARP MAC 00:00:00:00:00:11 to the Router ARP Table in index 0 */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac1, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfNhlfeArpLineIndex1, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table in index 1 */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac2, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfNhlfeArpLineIndex2, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfMplsNhlfeBasicConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE Configuration
*/
GT_VOID prvTgfMplsNhlfeBasicConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              numOfNhlfEntries=2;
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArray[2];
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArrayGet[2];

    PRV_UTF_LOG0_MAC("======= Setting NHLFE Basic Configuration =======\n");

    /* Create a new entries in the NHLFE table */

    nhlfEntryArray[0].nhlfeCommonParams.mplsLabel = 20;
    nhlfEntryArray[1].nhlfeCommonParams.mplsLabel = 21;

    nhlfEntryArray[0].nhlfeCommonParams.mplsCmd = PRV_TGF_MPLS_NOP_CMD_E;
    nhlfEntryArray[1].nhlfeCommonParams.mplsCmd = PRV_TGF_MPLS_NOP_CMD_E;

    nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.outlifType = PRV_TGF_OUTLIF_TYPE_LL_E;
    nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.outlifType = PRV_TGF_OUTLIF_TYPE_LL_E;

    nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr =prvTgfNhlfeArpLineIndex1;
    nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr =prvTgfNhlfeArpLineIndex2;

    nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.interfaceInfo.type = CPSS_INTERFACE_PORT_E;
    nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.interfaceInfo.type = CPSS_INTERFACE_PORT_E;

    nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
    nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum = prvTgfDevNum;

    nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];/* port 23*/
    nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];/* port 23*/

    nhlfEntryArray[0].nhlfeCommonParams.nextHopVlanId = 2;
    nhlfEntryArray[1].nhlfeCommonParams.nextHopVlanId = 2;

    nhlfEntryArray[0].nhlfeCommonParams.ageRefresh = GT_FALSE;
    nhlfEntryArray[1].nhlfeCommonParams.ageRefresh = GT_FALSE;

    nhlfEntryArray[0].nhlfeCommonParams.mtuProfileIndex = 0;
    nhlfEntryArray[1].nhlfeCommonParams.mtuProfileIndex = 0;

    nhlfEntryArray[0].nhlfeCommonParams.counterSetIndex = PRV_TGF_MPLS_COUNTER_SET_INDEX_0_E;
    nhlfEntryArray[1].nhlfeCommonParams.counterSetIndex = PRV_TGF_MPLS_COUNTER_SET_INDEX_0_E;

    nhlfEntryArray[0].nhlfeCommonParams.mirrorToIngressAnalyzer = GT_FALSE;
    nhlfEntryArray[1].nhlfeCommonParams.mirrorToIngressAnalyzer = GT_FALSE;

    nhlfEntryArray[0].nhlfeCommonParams.ttl = 10;
    nhlfEntryArray[1].nhlfeCommonParams.ttl = 20;

    nhlfEntryArray[0].nhlfeCommonParams.ttlMode = PRV_TGF_MPLS_TTL_DECREMENTED_BY_ENTRY_E;
    nhlfEntryArray[1].nhlfeCommonParams.ttlMode = PRV_TGF_MPLS_TTL_DECREMENTED_BY_ONE_E;

    nhlfEntryArray[0].nhlfeCommonParams.bypassTtlExceptionCheckEnable = GT_TRUE;
    nhlfEntryArray[1].nhlfeCommonParams.bypassTtlExceptionCheckEnable = GT_TRUE;

    nhlfEntryArray[0].nhlfeCommonParams.cpuCodeIndex = 0;
    nhlfEntryArray[1].nhlfeCommonParams.cpuCodeIndex = 1;

    nhlfEntryArray[0].nhlfeCommonParams.packetCmd = CPSS_PACKET_CMD_ROUTE_E;
    nhlfEntryArray[1].nhlfeCommonParams.packetCmd = CPSS_PACKET_CMD_ROUTE_E;

    nhlfEntryArray[0].qosParamsModify.modifyTc = GT_FALSE;
    nhlfEntryArray[1].qosParamsModify.modifyTc = GT_FALSE;

    nhlfEntryArray[0].qosParamsModify.modifyUp = GT_FALSE;
    nhlfEntryArray[1].qosParamsModify.modifyUp = GT_FALSE;

    nhlfEntryArray[0].qosParamsModify.modifyDp = GT_FALSE;
    nhlfEntryArray[1].qosParamsModify.modifyDp = GT_FALSE;

    nhlfEntryArray[0].qosParamsModify.modifyDscp = GT_FALSE;
    nhlfEntryArray[1].qosParamsModify.modifyDscp = GT_FALSE;

    nhlfEntryArray[0].qosParamsModify.modifyExp = GT_FALSE;
    nhlfEntryArray[1].qosParamsModify.modifyExp = GT_FALSE;

    nhlfEntryArray[0].qosParams.tc = 0;
    nhlfEntryArray[1].qosParams.tc = 0;

    nhlfEntryArray[0].qosParams.dp = CPSS_DP_GREEN_E;
    nhlfEntryArray[1].qosParams.dp = CPSS_DP_GREEN_E;

    nhlfEntryArray[0].qosParams.up = 0;
    nhlfEntryArray[1].qosParams.up = 0;

    nhlfEntryArray[0].qosParams.dscp = 0;
    nhlfEntryArray[1].qosParams.dscp = 0;

    nhlfEntryArray[0].qosParams.exp = 0;
    nhlfEntryArray[1].qosParams.exp = 0;

    /* AUTODOC: add 2 NHLFE entries baseIndex=16: */
    /* AUTODOC:   [0]: mplsLabel=20, mplsCmd=NOP */
    /* AUTODOC:        arpPtr=0, port=3, nextHopVlanId=2 */
    /* AUTODOC:        TTL=10, ttlMode=DECREMENTED_BY_ENTRY */
    /* AUTODOC:        cpuCodeIndex=0, packetCmd=ROUTE */
    /* AUTODOC:   [1]: mplsLabel=21, mplsCmd=NOP */
    /* AUTODOC:        arpPtr=1, port=3, nextHopVlanId=2 */
    /* AUTODOC:        TTL=20, ttlMode=DECREMENTED_BY_ONE */
    /* AUTODOC:        cpuCodeIndex=1, packetCmd=ROUTE */
    rc = prvTgfMplsNhlfEntriesWrite(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesWrite: %d", prvTgfDevNum);

    rc = prvTgfMplsNhlfEntriesRead(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArrayGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesRead: %d", prvTgfDevNum);

    /* Check fields */
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.mplsLabel,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsLabel,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.mplsLabel:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsLabel);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.mplsLabel,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsLabel,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.mplsLabel:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsLabel);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.outlifType,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifType,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifType:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifType);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.outlifType,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifType,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifType:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifType);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.outlifPointer.arpPtr);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.interfaceInfo.type,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.type,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.type:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.type);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.interfaceInfo.type,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.type,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.type:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.type);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.hwDevNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.outlifConfig.interfaceInfo.devPort.portNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.nextHopVlanId,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.nextHopVlanId,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.nextHopVlanId:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.nextHopVlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.nextHopVlanId,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.nextHopVlanId,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.nextHopVlanId:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.nextHopVlanId);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.ageRefresh,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ageRefresh,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.ageRefresh:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ageRefresh);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.ageRefresh,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ageRefresh,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.ageRefresh:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ageRefresh);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.mtuProfileIndex,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mtuProfileIndex,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.mtuProfileIndex:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mtuProfileIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.mtuProfileIndex,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mtuProfileIndex,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.mtuProfileIndex:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mtuProfileIndex);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.counterSetIndex,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.counterSetIndex,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.counterSetIndex:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.counterSetIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.counterSetIndex,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.counterSetIndex,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.counterSetIndex:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.counterSetIndex);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.mirrorToIngressAnalyzer,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mirrorToIngressAnalyzer,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.mirrorToIngressAnalyzer:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mirrorToIngressAnalyzer);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.mirrorToIngressAnalyzer,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mirrorToIngressAnalyzer,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.mirrorToIngressAnalyzer:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mirrorToIngressAnalyzer);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.ttl,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ttl,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.ttl:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ttl);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.ttl,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ttl,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.ttl:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ttl);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.ttlMode,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ttlMode,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.ttlMode:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ttlMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.ttlMode,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ttlMode,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.ttlMode:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ttlMode);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.bypassTtlExceptionCheckEnable,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.bypassTtlExceptionCheckEnable,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.bypassTtlExceptionCheckEnable:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.bypassTtlExceptionCheckEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.bypassTtlExceptionCheckEnable,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.bypassTtlExceptionCheckEnable,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.bypassTtlExceptionCheckEnable:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.bypassTtlExceptionCheckEnable);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.cpuCodeIndex,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.cpuCodeIndex,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.cpuCodeIndex:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.cpuCodeIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.cpuCodeIndex,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.cpuCodeIndex,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.cpuCodeIndex:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.cpuCodeIndex);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.packetCmd,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.packetCmd,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.packetCmd:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.packetCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.packetCmd,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.packetCmd,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.packetCmd:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.packetCmd);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParamsModify.modifyTc,
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyTc,
                                 "nhlfEntryArrayGet[0].qosParamsModify.modifyTc:%d",
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyTc);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParamsModify.modifyTc,
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyTc,
                                 "nhlfEntryArrayGet[1].qosParamsModify.modifyTc:%d",
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyTc);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParamsModify.modifyUp,
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyUp,
                                 "nhlfEntryArrayGet[0].qosParamsModify.modifyUp:%d",
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyUp);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParamsModify.modifyUp,
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyUp,
                                 "nhlfEntryArrayGet[1].qosParamsModify.modifyUp:%d",
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyUp);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParamsModify.modifyDp,
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyDp,
                                 "nhlfEntryArrayGet[0].qosParamsModify.modifyDp:%d",
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyDp);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParamsModify.modifyDp,
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyDp,
                                 "nhlfEntryArrayGet[1].qosParamsModify.modifyDp:%d",
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyDp);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParamsModify.modifyDscp,
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyDscp,
                                 "nhlfEntryArrayGet[0].qosParamsModify.modifyDscp:%d",
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyDscp);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParamsModify.modifyDscp,
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyDscp,
                                 "nhlfEntryArrayGet[1].qosParamsModify.modifyDscp:%d",
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyDscp);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParamsModify.modifyExp,
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyExp,
                                 "nhlfEntryArrayGet[0].qosParamsModify.modifyExp:%d",
                                 nhlfEntryArrayGet[0].qosParamsModify.modifyExp);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParamsModify.modifyExp,
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyExp,
                                 "nhlfEntryArrayGet[1].qosParamsModify.modifyExp:%d",
                                 nhlfEntryArrayGet[1].qosParamsModify.modifyExp);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParams.dp,
                                 nhlfEntryArrayGet[0].qosParams.dp,
                                 "nhlfEntryArrayGet[0].qosParams.dp:%d",
                                 nhlfEntryArrayGet[0].qosParams.dp);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParams.dp,
                                 nhlfEntryArrayGet[1].qosParams.dp,
                                 "nhlfEntryArrayGet[1].qosParams.dp:%d",
                                 nhlfEntryArrayGet[1].qosParams.dp);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParams.up,
                                 nhlfEntryArrayGet[0].qosParams.up,
                                 "nhlfEntryArrayGet[0].qosParams.up:%d",
                                 nhlfEntryArrayGet[0].qosParams.up);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParams.up,
                                 nhlfEntryArrayGet[1].qosParams.up,
                                 "nhlfEntryArrayGet[1].qosParams.up:%d",
                                 nhlfEntryArrayGet[1].qosParams.up);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParams.dscp,
                                 nhlfEntryArrayGet[0].qosParams.dscp,
                                 "nhlfEntryArrayGet[0].qosParams.dscp:%d",
                                 nhlfEntryArrayGet[0].qosParams.dscp);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParams.dscp,
                                 nhlfEntryArrayGet[1].qosParams.dscp,
                                 "nhlfEntryArrayGet[1].qosParams.dscp:%d",
                                 nhlfEntryArrayGet[1].qosParams.dscp);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].qosParams.exp,
                                 nhlfEntryArrayGet[0].qosParams.exp,
                                 "nhlfEntryArrayGet[0].qosParams.exp:%d",
                                 nhlfEntryArrayGet[0].qosParams.exp);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].qosParams.exp,
                                 nhlfEntryArrayGet[1].qosParams.exp,
                                 "nhlfEntryArrayGet[1].qosParams.exp:%d",
                                 nhlfEntryArrayGet[1].qosParams.exp);
}

/**
* @internal prvTgfMplsNhlfeSwapConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE SWAP Configuration
*/
GT_VOID prvTgfMplsNhlfeSwapConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              numOfNhlfEntries=2;
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArray[2];
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArrayGet[2];

    PRV_UTF_LOG0_MAC("======= Setting NHLFE Swap Configuration =======\n");

    /* Update the NHLFE table configured in prvTgfMplsNhlfeBasicConfigurationSet phase */

    rc = prvTgfMplsNhlfEntriesRead(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesRead: %d", prvTgfDevNum);

    nhlfEntryArray[0].nhlfeCommonParams.mplsCmd = PRV_TGF_MPLS_SWAP_CMD_E;

    /* AUTODOC: update NHLFE entry 0, baseIndex=16: */
    /* AUTODOC:   [0]: mplsCmd=SWAP */
    rc = prvTgfMplsNhlfEntriesWrite(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesWrite: %d", prvTgfDevNum);

    rc = prvTgfMplsNhlfEntriesRead(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArrayGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesRead: %d", prvTgfDevNum);

    /* Check field updated  */
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd);
}

/**
* @internal prvTgfMplsNhlfePushConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE PHP Configuration
*/
GT_VOID prvTgfMplsNhlfePushConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              numOfNhlfEntries=2;
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArray[2];
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArrayGet[2];

    PRV_UTF_LOG0_MAC("======= Setting NHLFE Push Configuration =======\n");

    /* Update the NHLFE table configured in prvTgfMplsNhlfeBasicConfigurationSet phase */

    rc = prvTgfMplsNhlfEntriesRead(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesRead: %d", prvTgfDevNum);

    nhlfEntryArray[1].nhlfeCommonParams.mplsCmd = PRV_TGF_MPLS_PUSH_CMD_E;

    /* AUTODOC: update NHLFE entry 1, baseIndex=16: */
    /* AUTODOC:   [1]: mplsCmd=PUSH */
    rc = prvTgfMplsNhlfEntriesWrite(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesWrite: %d", prvTgfDevNum);

    rc = prvTgfMplsNhlfEntriesRead(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArrayGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesRead: %d", prvTgfDevNum);

    /* Check field updated  */
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd);
}

/**
* @internal prvTgfMplsNhlfePhpConfigurationSet function
* @endinternal
*
* @brief   Set NHLFE PUSH Configuration
*/
GT_VOID prvTgfMplsNhlfePhpConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              numOfNhlfEntries=2;
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArray[2];
    PRV_TGF_MPLS_NHLFE_STC              nhlfEntryArrayGet[2];

    PRV_UTF_LOG0_MAC("======= Setting NHLFE Php Configuration =======\n");

    /* Update the NHLFE table configured in prvTgfMplsNhlfeBasicConfigurationSet phase */

    rc = prvTgfMplsNhlfEntriesRead(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesRead: %d", prvTgfDevNum);

    nhlfEntryArray[0].nhlfeCommonParams.mplsCmd = PRV_TGF_MPLS_PHP_CMD_E;
    nhlfEntryArray[0].nhlfeCommonParams.ttlMode = PRV_TGF_MPLS_TTL_COPY_ENTRY_E;

    /* AUTODOC: update NHLFE entry 0, baseIndex=16: */
    /* AUTODOC:   [0]: mplsCmd=PHP, ttlMode=COPY_ENTRY */
    rc = prvTgfMplsNhlfEntriesWrite(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesWrite: %d", prvTgfDevNum);

    rc = prvTgfMplsNhlfEntriesRead(prvTgfDevNum, nhlfEntryBaseIndex,
                                        numOfNhlfEntries,nhlfEntryArrayGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMplsNhlfEntriesRead: %d", prvTgfDevNum);

    /* Check field updated  */
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.mplsCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.mplsCmd,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.mplsCmd);

    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[0].nhlfeCommonParams.ttlMode,
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ttlMode,
                                 "nhlfEntryArrayGet[0].nhlfeCommonParams.ttlMode:%d",
                                 nhlfEntryArrayGet[0].nhlfeCommonParams.ttlMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(nhlfEntryArray[1].nhlfeCommonParams.ttlMode,
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ttlMode,
                                 "nhlfEntryArrayGet[1].nhlfeCommonParams.ttlMode:%d",
                                 nhlfEntryArrayGet[1].nhlfeCommonParams.ttlMode);
}

/**
* @internal prvTgfMplsBasicTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMplsBasicTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating MPLS Traffic =======\n");

   /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
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
            /* reset IP couters and set ROUTE_ENTRY mode for MPLS packets */
            rc = prvTgfMplsCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketMplsPartArray) / sizeof(prvTgfPacketMplsPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketMplsPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketMplsPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send MPLS packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:11:11 */
    /* AUTODOC:   MPLS EtherType=0x8847 */
    /* AUTODOC:   MPLS label=0x10, stack=1, ttl=0x40 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


   /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get MPLS packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:11:11, VID=2 */
    /* AUTODOC:   MPLS label=0x10, stack=1, ttl=0x40 - no change */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

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

                break;

            case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

                /* packetSize is changed due to added VLAN tag (4 bytes)*/
                expectedPacketSize = (packetSize + TGF_CRC_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

            default:

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

                break;
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
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfPacket1L2Part */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacket1L2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfPacket1L2Part.daMac[0], prvTgfPacket1L2Part.daMac[1],
            prvTgfPacket1L2Part.daMac[2],prvTgfPacket1L2Part.daMac[3],
            prvTgfPacket1L2Part.daMac[4], prvTgfPacket1L2Part.daMac[5]);

    /* check if captured packet has the expected vlan tag "81 00 00 02" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = 0x81;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x02;


    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n vlan tag captured packet must be: 81 00 00 02");

    /* check if captured packet has the expected MPLS params - label=0x10 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 16;
    vfdArray[0].cycleCount = 5;
    vfdArray[0].patternPtr[0] = 0x88;
    vfdArray[0].patternPtr[1] = 0x47;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x01;
    vfdArray[0].patternPtr[4] = 0x01;


    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n MPLS Label and ttl of captured packet must be: 88 47 00 01 01");


    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfMplsSwapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - Swap Label
*/
GT_VOID prvTgfMplsSwapTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating MPLS Swap Traffic =======\n");

   /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
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
            /* reset IP couters and set ROUTE_ENTRY mode for MPLS packets */
            rc = prvTgfMplsCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketMplsPartArray) / sizeof(prvTgfPacketMplsPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketMplsPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketMplsPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send MPLS packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:11:11 */
    /* AUTODOC:   MPLS EtherType=0x8847 */
    /* AUTODOC:   MPLS label=0x10, stack=1, ttl=0x40 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


   /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get MPLS packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:11, SA=00:00:00:00:00:02, VID=2 */
    /* AUTODOC:   MPLS label=0x14, stack=1, ttl=0x36 - decrement by entry */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

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

                break;

            case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

                /* packetSize is changed due to added VLAN tag (4 bytes)*/
                expectedPacketSize = (packetSize + TGF_CRC_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

            default:

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

                break;
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
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac1 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac1, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac1[0], prvTgfArpMac1[1],prvTgfArpMac1[2],prvTgfArpMac1[3],
            prvTgfArpMac1[4], prvTgfArpMac1[5]);

    /* check if captured packet has the expected MAC SA  " 00:00:00:00:00:02*/
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 6;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);;
    vfdArray[0].patternPtr[0] = 0x00;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x00;
    vfdArray[0].patternPtr[4] = 0x00;
    vfdArray[0].patternPtr[5] = 0x00;
    vfdArray[0].patternPtr[6] = 0x02;

    /* check if captured packet has the expected vlan tag "81 00 00 02" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = 0x81;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x02;


    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n vlan tag captured packet must be: 81 00 00 02");

    /* check if captured packet has the expected MPLS paramers: Label=0x14 ttl=0x36 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 16;
    vfdArray[0].cycleCount = 6;
    vfdArray[0].patternPtr[0] = 0x88;
    vfdArray[0].patternPtr[1] = 0x47;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x01;
    vfdArray[0].patternPtr[4] = 0x41;
    vfdArray[0].patternPtr[5] = 0x36;


    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n MPLS Label and ttl of captured packet must be: 88 47 00 01 41 36");


    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}


/**
* @internal prvTgfMplsPushTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - Push Label
*/
GT_VOID prvTgfMplsPushTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating MPLS Push Traffic =======\n");

   /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
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
            /* reset IP couters and set ROUTE_ENTRY mode for MPLS packets */
            rc = prvTgfMplsCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketMplsPartArray2) / sizeof(prvTgfPacketMplsPartArray2[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketMplsPartArray2, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketMplsPartArray2;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send MPLS packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:11:11 */
    /* AUTODOC:   MPLS EtherType=0x8847 */
    /* AUTODOC:   MPLS label=0x11, stack=1, ttl=0x40 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


   /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get MPLS packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:02, VID=2 */
    /* AUTODOC:   MPLS label=0x15, stack=1, ttl=0x3F - decrement by one */
    /* AUTODOC:   MPLS label=0x11, stack=1, ttl=0x3F - decrement by one */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

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

                break;

            case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

                /* packetSize is changed due to added VLAN tag (4 bytes), and added MPLS Label */
                expectedPacketSize = (packetSize + TGF_CRC_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

            default:

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

                break;
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
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac2 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac2, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac2[0], prvTgfArpMac2[1],prvTgfArpMac2[2],prvTgfArpMac2[3],
            prvTgfArpMac2[4], prvTgfArpMac2[5]);

    /* check if captured packet has the expected MAC SA  " 00:00:00:00:00:02*/
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 6;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);;
    vfdArray[0].patternPtr[0] = 0x00;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x00;
    vfdArray[0].patternPtr[4] = 0x00;
    vfdArray[0].patternPtr[5] = 0x00;
    vfdArray[0].patternPtr[6] = 0x02;

    /* check if captured packet has the expected vlan tag "81 00 00 02" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = 0x81;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x02;


    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n vlan tag captured packet must be: 81 00 00 02");

    /* check if captured packet has the expected MPLS paramers: 2 Label=0x15 0x11, ttl=0x3F */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 16;
    vfdArray[0].cycleCount = 10;
    vfdArray[0].patternPtr[0] = 0x88;
    vfdArray[0].patternPtr[1] = 0x47;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x01;
    vfdArray[0].patternPtr[4] = 0x50;
    vfdArray[0].patternPtr[5] = 0x3F;
    vfdArray[0].patternPtr[6] = 0x00;
    vfdArray[0].patternPtr[7] = 0x01;
    vfdArray[0].patternPtr[8] = 0x11;
    vfdArray[0].patternPtr[9] = 0x40;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n MPLS Label and ttl of captured packet must be: 88 47 00 01 50 3F 00 01 11 3F");


    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfMplsPhpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - Push Label
*/
GT_VOID prvTgfMplsPhpTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating MPLS PHP Traffic =======\n");

   /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
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
            /* reset IP couters and set ROUTE_ENTRY mode for MPLS packets */
            rc = prvTgfMplsCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketMplsPartArray3) / sizeof(prvTgfPacketMplsPartArray3[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketMplsPartArray3, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketMplsPartArray3;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send MPLS packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:11:11 */
    /* AUTODOC:   MPLS EtherType=0x8847 */
    /* AUTODOC:   MPLS_1 label=0x10, stack=1, ttl=0x40 */
    /* AUTODOC:   MPLS_2 label=0xA, stack=1, ttl=0x40 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


   /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get MPLS packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:00:11, SA=00:00:00:00:00:02, VID=2 */
    /* AUTODOC:   MPLS label=0xA, stack=1, ttl=0xA - copy entry */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

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

                break;

            case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

                /* packetSize is changed due to added VLAN tag (4 bytes), and removed MPLS Label */
                expectedPacketSize = (packetSize + TGF_CRC_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS - TGF_MPLS_HEADER_SIZE_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

            default:

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

                break;
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
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac1 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac1, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac1[0], prvTgfArpMac1[1],prvTgfArpMac1[2],prvTgfArpMac1[3],
            prvTgfArpMac1[4], prvTgfArpMac1[5]);

    /* check if captured packet has the expected MAC SA  " 00:00:00:00:00:02*/
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 6;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);;
    vfdArray[0].patternPtr[0] = 0x00;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x00;
    vfdArray[0].patternPtr[4] = 0x00;
    vfdArray[0].patternPtr[5] = 0x00;
    vfdArray[0].patternPtr[6] = 0x02;

    /* check if captured packet has the expected vlan tag "81 00 00 02" */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = 0x81;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x02;


    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n vlan tag captured packet must be: 81 00 00 02");

    /* check if captured packet has the expected MPLS paramers: Label=0xA, ttl=0xA */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 16;
    vfdArray[0].cycleCount = 6;
    vfdArray[0].patternPtr[0] = 0x88;
    vfdArray[0].patternPtr[1] = 0x47;
    vfdArray[0].patternPtr[2] = 0x00;
    vfdArray[0].patternPtr[3] = 0x00;
    vfdArray[0].patternPtr[4] = 0xA1;
    vfdArray[0].patternPtr[5] = 0x0A;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,"\n MPLS Label and ttl of captured packet must be: 88 47 00 00 A1 0A");


    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfMplsConfigurationRestore function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
GT_VOID prvTgfMplsConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
    * 1. Restore Route Configuration
    */

    /* AUTODOC: restore default inlif port mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_SEND_PORT_IDX_CNS,inlifMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfInlifPortModeSet");

    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryInvalidate");
};

