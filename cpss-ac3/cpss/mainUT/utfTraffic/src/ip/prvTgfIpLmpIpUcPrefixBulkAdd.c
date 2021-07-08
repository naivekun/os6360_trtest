/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfIpLpmIpUcPrefixBulkAdd.c
*
* DESCRIPTION:
*       IP UC Prefix Bulk Add/Delete enhanced test.
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*
*******************************************************************************/

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
#include <ip/prvTgfIpLpmIpUcPrefixBulkAdd.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* default index of send port in prvTgfPortsArray */
#define PRV_TGF_DEF_SEND_PORT_IDX_CNS     0
/* default send port number */
#define PRV_TGF_DEF_SEND_PORT_CNS  prvTgfPortsArray[PRV_TGF_DEF_SEND_PORT_IDX_CNS]

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS,
                                          PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x22, 0x00};

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/******************************* Test packets *********************************/
/*
 * ===========================================================================
 * ================ Common Ethernet frame part declaration ===================
 * ===========================================================================
 */

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* VLAN_TAG part for L2 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* IPv4 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartIpv4 =
                                            {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* IPv6 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartIpv6 =
                                            {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/*
 * ===========================================================================
 * ================ IPv4 headers declaration =================================
 * ===========================================================================
 */
#define  PRV_TGF_IPV4_DEF_DST_ADDR   { 192,  168,  0,  5}

static TGF_IPV4_ADDR ipv4DstAddr = PRV_TGF_IPV4_DEF_DST_ADDR;

/* Default IPv4 header to be inserted in IPv4 packet during dynamic shaping */
static TGF_PACKET_IPV4_STC prvTgfDefHeaderIpv4 = {
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
    PRV_TGF_IPV4_DEF_DST_ADDR   /* dstAddr */
};

/*
 * ===========================================================================
 * ================ IPv6 headers declaration =================================
 * ===========================================================================
 */
#define  PRV_TGF_IPV6_DEF_DST_ADDR   {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}

static TGF_IPV6_ADDR ipv6DstAddr = PRV_TGF_IPV6_DEF_DST_ADDR;

/* Default IPv6 header to be inserted in IPv6 packet during dynamic shaping */
static TGF_PACKET_IPV6_STC prvTgfDefHeaderIpv6 = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    PRV_TGF_IPV6_DEF_DST_ADDR                /* TGF_IPV6_ADDR dstAddr */
};


/*
 * ===========================================================================
 * ================ IPv4 packets declaration =================================
 * ===========================================================================
 */
/* DATA of Ipv4 packets */
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

/* Default Ipv4 packet construction, will be reshaped in traffic generation */
static TGF_PACKET_PART_STC prvTgfDefPacketIpv4[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartIpv4},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfDefHeaderIpv4},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*
 * ===========================================================================
 * ================ IPv6 packets declaration =================================
 * ===========================================================================
 */

/* DATA of Ipv6 packet */
static GT_U8 prvTgfPayloadDataArrIpv6[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPartIpv6 =
{
    sizeof(prvTgfPayloadDataArrIpv6),                   /* dataLength */
    prvTgfPayloadDataArrIpv6                            /* dataPtr */
};

/* 1st Ipv6 packet to check full match prefix:
 * - {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}/128*/
static TGF_PACKET_PART_STC prvTgfDefPacketIpv6[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartIpv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfDefHeaderIpv6},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPartIpv6}
};

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_INLIF_PORT_MODE_ENT                 sendPortInlifMode;
    GT_U16                                      vid;
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_INLIF_ENTRY_STC                 inlifSave;
#endif /* EXMXPM_FAMILY */
} prvTgfRestoreCfg;

/* number of IPvX prefixes to use during enhanced UT */
static GT_U32      prvTgfIpLpmBulkTestsCntr = 0;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U8                       tagArray[]      = {0, 1, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry        = {{0}};

    GT_U32           seed           = 0; /* seed for random generator */

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");
    /* clear bulk tests counter*/
    prvTgfIpLpmBulkTestsCntr = 0;

    /* get random seed */
    seed = prvUtfSeedFromStreamNameGet();
    /* set specific seed for random generator */
    cpssOsSrand(seed);

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_DEF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgVlanPortVidGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_DEF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vid);

    /* AUTODOC: create VLAN 5 with untagged port 0 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray, NULL,
                                           tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 1,2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
                                           prvTgfPortsArray + 1, NULL,
                                           tagArray + 1, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgVlanLearningStateSet: %d",
                                 prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * Set the FDB entry With DA_ROUTE
     */

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   =
                                prvTgfPortsArray[PRV_TGF_DEF_SEND_PORT_IDX_CNS];
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

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 0 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] qtyOfRouteEntries        - number of route entries to add during configuration
*                                       None
*/
static GT_VOID prvTgfIpLpmIpUcPrefixBulkAddLttRouteConfigurationSet
(
    GT_U32 qtyOfRouteEntries
)
{
    GT_STATUS                        rc                         = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC          inlifEntry                 = {0};
    GT_ETHERADDR                     arpMacAddr                 = {{0}};
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC    ipvxUCRouteEntrySet        = {0};
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC    ipvxUCRouteEntryGet        = {0};

    GT_U32    i = 0; /* iterator */

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
     */

    /* save current port inlif mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_DEF_SEND_PORT_IDX_CNS,
                                &(prvTgfRestoreCfg.sendPortInlifMode));

    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeGet);

    /* set port inlif mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_DEF_SEND_PORT_IDX_CNS,
                                PRV_TGF_INLIF_PORT_MODE_PORT_E);

    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);


#ifdef EXMXPM_FAMILY
    /* for puma devices store whole info about inlif, to restore after */
    rc = cpssExMxPmInlifEntryGet(0, PRV_TGF_INLIF_TYPE_PORT_E,
                                    PRV_TGF_DEF_SEND_PORT_IDX_CNS,
                                   &(prvTgfRestoreCfg.inlifSave));
#endif

    cpssOsMemSet(&inlifEntry,0,sizeof(inlifEntry));
    inlifEntry.bridgeEnable                   = GT_TRUE;
    inlifEntry.unkSaUcCommand                 = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipv4UcRouteEnable              = GT_TRUE;
    inlifEntry.ipv6UcRouteEnable              = GT_TRUE;


    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_PORT_E,
                             PRV_TGF_DEF_SEND_PORT_IDX_CNS, &inlifEntry);

    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifEntrySet);

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4/Ipv6 Routing on port send port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_DEF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_DEF_SEND_PORT_IDX_CNS);

    /* AUTODOC: enable IPv4/Ipv6 Unicast Routing on send Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 2. Create route entries in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC[00:00:00:00:00:22..22+qtyOfRouteEntries]
                to the Router ARP Table.
     */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));
    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(&ipvxUCRouteEntrySet.cmd, 0, sizeof(ipvxUCRouteEntrySet));

    ipvxUCRouteEntrySet.cmd                      = CPSS_PACKET_CMD_ROUTE_E;
    ipvxUCRouteEntrySet.countSet                 = PRV_TGF_COUNT_SET_CNS;
    ipvxUCRouteEntrySet.nextHopVlanId            = PRV_TGF_NEXTHOPE_VLANID_CNS;
    ipvxUCRouteEntrySet.nextHopInterface.type    = CPSS_INTERFACE_PORT_E;
    ipvxUCRouteEntrySet.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;

    for (i=0; i< qtyOfRouteEntries; i++)
    {
        arpMacAddr.arEther[5] = (GT_U8)i;
        ipvxUCRouteEntrySet.nextHopARPPointer = prvTgfRouterArpIndex + i;

        rc = prvTgfIpRouterArpAddrWrite(ipvxUCRouteEntrySet.nextHopARPPointer,
                                        &arpMacAddr);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                    "prvTgfIpRouterArpAddrWrite: %d",
                                     prvTgfDevNum);

        /* AUTODOC: circular assignment of nexthop ports to route entries
                    1,4,7... - to port 1 from prvTgfPortsArray[];
                    2,5,8... - to port 2 from prvTgfPortsArray[];
                    3,6,9... - to port 3 from prvTgfPortsArray[].
         */
        ipvxUCRouteEntrySet.nextHopInterface.devPort.portNum =
                                                 prvTgfPortsArray[(i % 3) + 1];

        /* AUTODOC: add UC route entry with nexthop VLAN 6 and ports[1,2,3] */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex + i,
                                         &ipvxUCRouteEntrySet, 1);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                    "prvTgfIpUcRouteEntriesWrite: %d",
                                     prvTgfDevNum);

        /* AUTODOC: read and check the UC Route entry from the Route Table */
        cpssOsMemSet((GT_VOID*)&ipvxUCRouteEntryGet, 0,
                         sizeof(ipvxUCRouteEntryGet));

        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum,
                                        prvTgfRouteEntryBaseIndex + i,
                                        &ipvxUCRouteEntryGet, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                    "prvTgfIpUcRouteEntriesRead: %d",
                                     prvTgfDevNum);

        PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                          ipvxUCRouteEntryGet.nextHopVlanId,
                          ipvxUCRouteEntryGet.nextHopInterface.devPort.portNum);
    }
}

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddPbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
static GT_VOID prvTgfIpLpmIpUcPrefixBulkAddPbrConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");
    /* -------------------------------------------------------------------------
     * 1. PCL Config
     */

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(prvTgfPortsArray[PRV_TGF_DEF_SEND_PORT_IDX_CNS],
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    CPSS_PCL_LOOKUP_1_E, /* PBR must be in lookup 1 ! */
                    PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc,
                                 "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* enable IPv4/Ipv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_TRUE);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] qtyOfRouteEntries        - number of route entries to add during configuration
*                                       None
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddRouteConfigurationSet
(
    GT_U32 qtyOfRouteEntries
)
{
    GT_STATUS                       rc          = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode =
                                        PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            prvTgfIpLpmIpUcPrefixBulkAddPbrConfigurationSet();
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            prvTgfIpLpmIpUcPrefixBulkAddLttRouteConfigurationSet(qtyOfRouteEntries);
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}
/**
* @internal prvTgfIpLpmIpvxUcPrefixBulkCheckParams function
* @endinternal
*
* @brief   Check input protocol validity.
*
* @param[in] prot                     - protocol to to be checked [CPSS_IP_PROTOCOL_IPV4_E/
*                                      CPSS_IP_PROTOCOL_IPV6_E]
*                                       None
*/
static GT_STATUS prvTgfIpLpmIpvxUcPrefixBulkCheckParams
(
    CPSS_IP_PROTOCOL_STACK_ENT  prot
)
{
    switch(prot)
    {
        case CPSS_IP_PROTOCOL_IPV4_E :
        case CPSS_IP_PROTOCOL_IPV6_E :
            break;
        case CPSS_IP_PROTOCOL_IPV4V6_E:
        default:
        {
            PRV_UTF_LOG1_MAC("Protocol %d is not supported \n", prot);
            return GT_BAD_PARAM;
        }
    };

    return GT_OK;
}
/**
* @internal prvTgfIpLpmIpvxUcPrefixBulkTest function
* @endinternal
*
* @brief   Run LPM bulk prefixes add/delete test with complex scenario.
*         The goal is to add/delete IP prefixes to LPM table bucket and to
*         transform it through all hierarchy levels (compressed 1, compressed 2,
*         regular) while checking routing capabilities of LPM.
* @param[in] ipvxProtVer              - protocol to run test for [ipv4,ipv6]
*                                       None
*/
GT_VOID prvTgfIpLpmIpvxUcPrefixBulkTest
(
    CPSS_IP_PROTOCOL_STACK_ENT  ipvxProtVer
)
{

    GT_U32           i              = 0; /* iterator */
    GT_U32           ii             = 0; /* iterator */
    GT_U32           iii            = 0; /* iterator */
    GT_U32           prefixesLen    = 0; /* temp variable*/
    GT_U8            bcktL          = 1; /* LPM bucket index for the test */
    GT_U8            bcktM          = 4; /* Maximum bucket layer level */
    GT_U8            bcktX          = 0; /* X variable from scenario */
    GT_U8            bcktY          = 0; /* Y variable from scenario */
    GT_U8            L0Octet        = 0; /* L0 octet to generate randomly */

    GT_STATUS        rc             = GT_TRUE;

    PRV_TGF_IP_ROUTING_MODE_ENT routingMode =
                                        PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E;
    PRV_TGF_IPVX_LPM_PREFIXES_BULK_STORE_STC prvTgfIpvxLpmPrefixesBulkStoreStc =
                                                                        {0};

    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nhPtr;

    /* check if proper protocol is entered */
    rc = prvTgfIpLpmIpvxUcPrefixBulkCheckParams(ipvxProtVer);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "Bad input parameters: %d", ipvxProtVer);



    /* clean general private store structure */
    cpssOsMemSet(&prvTgfIpvxLpmPrefixesBulkStoreStc, 0,
                 sizeof(prvTgfIpvxLpmPrefixesBulkStoreStc));

    /* Scenario Paragraph 1. Set up maximum level in prefix to be tested */
    bcktM = (CPSS_IP_PROTOCOL_IPV4_E == ipvxProtVer) ? 4 : 16;

    /* Scenario Paragraph 2. Select a bucket level for the test randomly
     * in range [ 1<= L < M ];
     */
    bcktL = (GT_U8)(cpssOsRand() % bcktM);
    if (bcktL == 0) bcktL = 1; /* to keep in range */
    PRV_UTF_LOG1_MAC("\nRandomly selected a bucket level in range [ 1<= L < M ]"
                     " :%d\n", bcktL);

    /* Scenario Paragraph 4. Select a prefix length PL randomly
     * in range [8*(L+1) <= P <=8*M];
     */
    prefixesLen = 8*(bcktL+1);
    if (0 != (bcktM - bcktL - 1))
    {
        prefixesLen += (GT_U8)(cpssOsRand() % ((bcktM - bcktL - 1)*8));
    }
    PRV_UTF_LOG1_MAC("Randomly selected a prefix length PL in range "
                     "[8*(L+1) <= P <=8*M]: %d\n", prefixesLen);

    /* fill prefix length arrays by new randomly generated prefixesLen */
    for (ii = 0; ii < PRV_TGF_MAX_PREFIXES_TO_ADD_CNS; ii++)
    {
        switch (ipvxProtVer)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[ii] = prefixesLen;
                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[ii] = prefixesLen;
                break;
            default : break;
        }
    }

    /* Scenario Paragraph 5. Select random number X in range [0 < X < 253];
     */
    bcktX = (GT_U8)(cpssOsRand() % 253);
    if (0 == bcktX) bcktX = 1; /* to keep in range */
    PRV_UTF_LOG1_MAC("Randomly selected number X in range [0 < X < 253]: %d\n",
                                                                        bcktX);

    /* Scenario Paragraph 6. Select random number Y in range [1 < Y < (255-X)];
     */
    bcktY = (GT_U8)(cpssOsRand() % (255-bcktX));
    if ((0 == bcktY) || (1 == bcktY)) bcktY = 2; /* to keep in range */
    PRV_UTF_LOG1_MAC("Randomly selected number Y in range [1 < Y < (255-X)]:"
                     " %d\n\n", bcktY);

    /* Scenario Paragraph 3. Fill octets for the rest of the levels [M-1] */
    /* Scenario Paragraph 7. Add a bulk of 3 IP prefixes
            (to force the bucket at selected level L to be changed from
            compressed 1 to compressed 2):
            1) for 1st IP address use X for the octet at level L,
                  point prefix to -> NH = 1.
            2) for 2nd IP address use X+Y for the octet at level L,
                  point prefix to -> NH = 2.
            3) for 3rd IP address use (X+Y+1) for the octet at level L,
                  point prefix to -> NH = 3.
    */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* init next hope info structure */
    for (i=0; i < PRV_TGF_MAX_PREFIXES_TO_ADD_CNS; i++)
    {
        /* temp pointer for easy code reading*/
        nhPtr = &prvTgfIpvxLpmPrefixesBulkStoreStc.nextHopInfoPtr[i];

        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
                nhPtr->ipLttEntry.routeEntryBaseIndex =
                                                  prvTgfRouteEntryBaseIndex + i;
                nhPtr->ipLttEntry.numOfPaths = 0;
                nhPtr->ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
                break;
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
                nhPtr->pclIpUcAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
                nhPtr->pclIpUcAction.mirror.cpuCode = CPSS_NET_CONTROL_E;
                nhPtr->pclIpUcAction.vlan.vlanId = PRV_TGF_NEXTHOPE_VLANID_CNS;
                nhPtr->pclIpUcAction.ipUcRoute.arpDaIndex =
                                                      prvTgfRouterArpIndex + i;
                nhPtr->pclIpUcAction.redirect.redirectCmd =
                                      PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
                nhPtr->pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
                nhPtr->pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
                nhPtr->pclIpUcAction.redirect.data.outIf.outInterface.type =
                                                          CPSS_INTERFACE_PORT_E;
                nhPtr->pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum =
                                                                  prvTgfDevNum;
                /* AUTODOC:
                         circular assignment of nexthop ports to route entries:
                           1,4,7... - to port 1 from prvTgfPortsArray[];
                           2,5,8... - to port 2 from prvTgfPortsArray[];
                           3,6,9... - to port 3 from prvTgfPortsArray[].
                */
                nhPtr->pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum =
                                                  prvTgfPortsArray[(i % 3) + 1];
                break;
            default: break;
        }

        /* init a destination IP addresses for the prefix from default
         * packet destination address */
        switch (ipvxProtVer)
        {
            case CPSS_IP_PROTOCOL_IPV4_E :
                cpssOsMemCpy(&(prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[i].arIP),
                               ipv4DstAddr, sizeof(TGF_IPV4_ADDR));

                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                for (ii = 0; ii < 8; ii++)
                {
                    prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[i].arIP[ii*2] =
                                                 (GT_U8)(ipv6DstAddr[ii] >> 8);
                    prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[i].arIP[ii*2+1]=
                                                 (GT_U8)ipv6DstAddr[ii];
                }
                break;
            default: break;
        }
    }
    /* store Added Prefixes Q-ty */
    prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty = 3;

    switch (ipvxProtVer)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* 7.1 for 1st IP address use X for the octet at level L   */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0].arIP[bcktL] = bcktX;
            /* 7.2 for 2nd IP address use X+Y for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[1].arIP[bcktL] = bcktX +
                                                                        bcktY;
            /* 7.3 for 3rd IP address use (X+Y+1) for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2].arIP[bcktL] = bcktX +
                                                                     bcktY + 1;
            rc = prvTgfIpLpmIpv4UcPrefixBulkAdd(prvTgfLpmDBId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.vrId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.nextHopInfoPtr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.override,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmIpv4UcBulkPrefixAdd: %d",
                                         prvTgfDevNum);
            if (GT_OK == rc)
            {
                PRV_UTF_LOG2_MAC("\nAdded %d IPv4 prefixes of %d prefix lengths:"
                                 " \n",
                                 prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty,
                                 prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[0]);
                for (iii=0; iii < prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty; iii++)
                {
                    PRV_UTF_LOG5_MAC("%d Prefix: %d:%d:%d:%d\n", iii+1,
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[1],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[3]);
                }
                PRV_UTF_LOG0_MAC("\n");
            }
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            /* 7.1 for 1st IP address use X for the octet at level L   */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0].arIP[bcktL] = bcktX;
            /* 7.2 for 2nd IP address use X+Y for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[1].arIP[bcktL] = bcktX +
                                                                        bcktY;
            /* 7.3 for 3rd IP address use (X+Y+1) for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2].arIP[bcktL] = bcktX +
                                                                     bcktY + 1;
            rc = prvTgfIpLpmIpv6UcPrefixBulkAdd(prvTgfLpmDBId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.vrId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.nextHopInfoPtr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.override,
                               GT_FALSE,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmIpv6UcBulkPrefixAdd: %d",
                                          prvTgfDevNum);
             if (GT_OK == rc)
             {
                 PRV_UTF_LOG2_MAC("\nAdded %d IPv6 prefixes of %d prefix lengths:"
                                  " \n",
                                  prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty,
                                  prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[0]);
                 for (iii=0; iii < prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty; iii++)
                 {
                     PRV_UTF_LOG5_MAC("%d Prefix: %x:%x:%x:%x\n", iii+1,
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[0],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[1],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[2],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[3]);
                 }
                 PRV_UTF_LOG0_MAC("\n");
             }
            break;
        default: break;
    }
    /*Scenario Paragraph 8. Send traffic for the 3 prefixes, check that
     *                      it reaches its destination
     */
    prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate(ipvxProtVer, 3,
                                            &prvTgfIpvxLpmPrefixesBulkStoreStc);

    /* Scenario Paragraph 9. Bulk delete 3rd prefix (to force octet at level L
     *                       to be changed back to compressed 1).
     */
    prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty--;
    switch (ipvxProtVer)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            rc = prvTgfIpLpmIpv4UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[2],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[2],
                               1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmIpv4UcBulkPrefixDel: %d",
                                         prvTgfDevNum);
            if (GT_OK == rc)
            {
                PRV_UTF_LOG5_MAC("\nDeleted 3rd IPv4 prefix of %d prefix length:"
                                 " %d:%d:%d:%d \n\n",
                                 prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2].arIP[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2].arIP[1],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2].arIP[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2].arIP[3]);
                PRV_UTF_LOG0_MAC("\n");
            }
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            rc = prvTgfIpLpmIpv6UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[2],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[2],
                               1);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmIpv6UcBulkPrefixDel: %d",
                                          prvTgfDevNum);
             if (GT_OK == rc)
             {
                 PRV_UTF_LOG5_MAC("\nDeleted 3rd IPv6 prefix of %d prefix length:"
                                  "%x:%x:%x:%x \n\n",
                                  prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[2],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2].u32Ip[0],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2].u32Ip[1],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2].u32Ip[2],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2].u32Ip[3]);

             }
            break;
        default: break;
    }
    /* Scenario Paragraph 10. Send traffic for 1st and 2nd prefixes, check
                              that it reaches its destinations.
                              Send traffic for 3rd prefix and check
                              that it is dropped.
    */
    prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate(ipvxProtVer, 3,
                                            &prvTgfIpvxLpmPrefixesBulkStoreStc);

    /* bulk delete rest 2 prefixes left */
    prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty = 0;

    switch (ipvxProtVer)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            rc = prvTgfIpLpmIpv4UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[0],
                               2);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmIpv4UcBulkPrefixDel: %d",
                                         prvTgfDevNum);
            if (GT_OK == rc)
            {
                PRV_UTF_LOG9_MAC("\nDeleted rest 2 IPv4 prefixes of %d prefix "
                                 "lengths:\n %d:%d:%d:%d\n %d:%d:%d:%d\n\n",
                                 prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0].arIP[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0].arIP[1],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0].arIP[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0].arIP[3],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[1].arIP[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[1].arIP[1],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[1].arIP[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[1].arIP[3]);
            }
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            rc = prvTgfIpLpmIpv6UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[0],
                               2);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmIpv6UcBulkPrefixDel: %d",
                                          prvTgfDevNum);
             if (GT_OK == rc)
             {
                 PRV_UTF_LOG9_MAC("\nDeleted rest 2 IPv6 prefixes of %d prefix "
                                  "length:\n %d:%d:%d:%d\n %d:%d:%d:%d\n\n",
                                  prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[0],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0].u32Ip[0],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0].u32Ip[1],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0].u32Ip[2],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0].u32Ip[3],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[1].u32Ip[0],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[1].u32Ip[1],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[1].u32Ip[2],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[1].u32Ip[3]);
             }
            break;
        default: break;
    }

    /* Scenario Paragraph 11. Select randomly a different octet value for L0.*/
    /* Limited L0Octet by 223 (IPv4 Class C maximum range)*/
    PRV_UTF_LOG0_MAC("Scenario Paragraph 11. Will add 5 new prefixes.\n");
    L0Octet = (GT_U8)(cpssOsRand() % 223);
    if (0 == L0Octet) L0Octet = 1;
    PRV_UTF_LOG1_MAC("Randomly selected a different octet value for L0: %d\n",
                                                                      L0Octet);

    /* Scenario Paragraph 12. Add a bulk of 3 IP prefixes (to force the bucket
                              at selected level L to be changed from
                              compressed 1 to compressed 2):
       1) use 250 for the octet at level L,
          point prefix to -> NH = 1.
       2) use 252 for the octet at level L,
          point prefix to -> NH = 2.
       3) use 254 for the octet at level L
          point prefix to -> NH = 3.
    */
    prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty = 3;

    switch (ipvxProtVer)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* 12.1 for 1st IP address use 250 for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0].arIP[bcktL] = 250;
            /* 12.2 for 2nd IP address use 252 for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[1].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[1].arIP[bcktL] = 252;
            /* 12.3 for 3rd IP address use 254 for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[2].arIP[bcktL] = 254;
            rc = prvTgfIpLpmIpv4UcPrefixBulkAdd(prvTgfLpmDBId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.vrId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.nextHopInfoPtr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.override,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmIpv4UcBulkPrefixAdd: %d",
                                         prvTgfDevNum);
            if (GT_OK == rc)
            {
                PRV_UTF_LOG2_MAC("\nAdded %d NEW IPv4 prefixes of %d prefix "
                                 "lengths: \n",
                                 prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty,
                                 prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[0]);
                for (iii=0; iii < prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty; iii++)
                {
                    PRV_UTF_LOG5_MAC("%d Prefix: %d:%d:%d:%d\n", iii+1,
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[1],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[3]);
                }
                PRV_UTF_LOG0_MAC("\n");
            }
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            /* 12.1 for 1st IP address use 250 for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0].arIP[bcktL] = 250;
            /* 12.2 for 2nd IP address use 252 for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[1].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[1].arIP[bcktL] = 252;
            /* 12.3 for 2nd IP address use 254 for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[2].arIP[bcktL] = 254;
            rc = prvTgfIpLpmIpv6UcPrefixBulkAdd(prvTgfLpmDBId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.vrId,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.nextHopInfoPtr,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.override,
                               GT_FALSE,
                               prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmIpv6UcBulkPrefixAdd: %d",
                                          prvTgfDevNum);
             if (GT_OK == rc)
             {
                 PRV_UTF_LOG2_MAC("\nAdded %d NEW IPv6 prefixes of %d prefix "
                                  "lengths: \n",
                                  prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty,
                                  prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[0]);
                 for (iii=0; iii < prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty; iii++)
                 {
                     PRV_UTF_LOG5_MAC("%d Prefix: %x:%x:%x:%x\n", iii+1,
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[0],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[1],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[2],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[3]);
                 }
                 PRV_UTF_LOG0_MAC("\n");
             }
            break;
        default: break;
    }

    /* Scenario Paragraph 13. Select random number X in range [0 < X < 247];
     */
    bcktX = (GT_U8)(cpssOsRand() % 247);
    if (0 == bcktX) bcktX = 1; /* to keep in range */
    PRV_UTF_LOG1_MAC("Randomly selected NEW number X in range [0 < X < 247]: "
                     "%d\n", bcktX);

    /* Scenario Paragraph 14. Select random number Y: Y=X+2;
     */
    bcktY = bcktX + 2;
    PRV_UTF_LOG1_MAC("Randomly selected NEW random number Y: Y=X+2: %d\n",
                                                                        bcktY);
    /* Scenario Paragraph 15. Add bulk of 2 IP prefixes (to force octet at
                           level L to be changed from compressed 2 to regular):
                            1) use X for the octet at level L,
                               point prefix to -> NH = 4.
                            2) use Y for the octet at level L
                               point prefix to -> NH = 5.
    */

    /* increase ipvx Added Prefixes Quantity by 2 */
    prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty += 2;

    switch (ipvxProtVer)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            /* 15.1 for 1st IP address use X for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[3].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[3].arIP[bcktL] = bcktY;
            /* 15.2 for 2nd IP address use Y for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[4].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[4].arIP[bcktL] = bcktX;
            rc = prvTgfIpLpmIpv4UcPrefixBulkAdd(prvTgfLpmDBId,
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.nextHopInfoPtr[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.override[3],
                           2);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmIpv4UcBulkPrefixAdd: %d",
                                         prvTgfDevNum);
            if (GT_OK == rc)
            {
                PRV_UTF_LOG1_MAC("\nAdded 2 more NEW IPv4 prefixes of %d "
                                 "prefixes lengths: \n",
                                 prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[0]);

                for (iii=3; iii < prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty; iii++)
                {
                    PRV_UTF_LOG5_MAC("%d Prefix: %d:%d:%d:%d\n", iii+1,
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[1],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[iii].arIP[3]);
                }
                PRV_UTF_LOG0_MAC("\n");
            }
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            /* 15.1 for 1st IP address use X for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[3].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[3].arIP[bcktL] = bcktY;
            /* 15.2 for 2nd IP address use Y for the octet at level L */
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[4].arIP[0] = L0Octet;
            prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[4].arIP[bcktL] = bcktX;
            rc = prvTgfIpLpmIpv6UcPrefixBulkAdd(prvTgfLpmDBId,
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.nextHopInfoPtr[3],
                           &prvTgfIpvxLpmPrefixesBulkStoreStc.override[3],
                           GT_FALSE,
                           2);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmIpv6UcBulkPrefixAdd: %d",
                                          prvTgfDevNum);
             if (GT_OK == rc)
             {
                 PRV_UTF_LOG1_MAC("\nAdded 2 more NEW IPv6 prefixes of %d "
                                  "prefixes lengths: \n",
                                  prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[0]);
                 for (iii=3; iii < prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty; iii++)
                 {
                     PRV_UTF_LOG5_MAC("%d Prefix: %x:%x:%x:%x\n", iii+1,
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[0],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[1],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[2],
                      prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[iii].u32Ip[3]);
                 }
                 PRV_UTF_LOG0_MAC("\n");
             }
            break;
        default: break;
    }

    /* Scenario Paragraph 16. Send traffic for the 5 prefixes and check that
     *                        it reaches its destination.
     */
    prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate(ipvxProtVer, 5,
                                            &prvTgfIpvxLpmPrefixesBulkStoreStc);

    /* Scenario Paragraph 17. Bulk delete the prefix from 15.1 (to force the
                         bucket at level L to be changed back to compressed 2).
    */

    prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty--;

    switch (ipvxProtVer)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            rc = prvTgfIpLpmIpv4UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[4],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[4],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[4],
                               1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmIpv4UcBulkPrefixDel: %d",
                                         prvTgfDevNum);
            if (GT_OK == rc)
            {
                PRV_UTF_LOG4_MAC("\nDeleted 5th prefix from recently "
                                 "added IPv4 prefixes: %d:%d:%d:%d\n\n",
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[4].arIP[0],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[4].arIP[1],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[4].arIP[2],
                       prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[4].arIP[3]);
            }
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            rc = prvTgfIpLpmIpv6UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[4],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[4],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[4],
                               1);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmIpv6UcBulkPrefixDel: %d",
                                          prvTgfDevNum);
             if (GT_OK == rc)
             {
                 PRV_UTF_LOG4_MAC("\nDeleted 5th prefix from recently "
                                  "added IPv6 prefixes: %x:%x:%x:%x\n\n",
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[4].u32Ip[0],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[4].u32Ip[1],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[4].u32Ip[2],
                        prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[4].u32Ip[3]);
             }
            break;
        default: break;
    }

    /* Scenario Paragraph 18. Send traffic again for the 5 prefixes.
                              Check that traffic to the prefix at 15.1 is
                              dropped, while traffic to the other 4 prefixes
                              reaches its destination.
    */
    prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate(ipvxProtVer, 5,
                                            &prvTgfIpvxLpmPrefixesBulkStoreStc);

    /* End of the scenario, bulk delete rest 4 prefixes */
    prvTgfIpvxLpmPrefixesBulkStoreStc.ipvxAPQty = 0;

    switch (ipvxProtVer)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            rc = prvTgfIpLpmIpv4UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4Addr[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv4PL[0],
                               4);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmIpv4UcBulkPrefixDel: %d",
                                         prvTgfDevNum);
            if (GT_OK == rc)
            {
                PRV_UTF_LOG0_MAC("\nIPv4 protocol test FINISHED. "
                                 "Deleted all recently added prefixes. \n");
            }
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            rc = prvTgfIpLpmIpv6UcPrefixBulkDel(prvTgfLpmDBId,
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.vrId[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6Addr[0],
                               &prvTgfIpvxLpmPrefixesBulkStoreStc.ipv6PL[0],
                               4);
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpLpmIpv6UcBulkPrefixDel: %d",
                                          prvTgfDevNum);
             if (GT_OK == rc)
             {
                 PRV_UTF_LOG0_MAC("\nIPv6 protocol test FINISHED. "
                                  "Deleted all recently added prefixes. \n");
             }
            break;
        default: break;
    }
}



/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] ipvxProtVer              - protocol to generate traffic for [ipv4,ipv6].
* @param[in] ipvxPrfxsQty             - number of prefixes to send packets for.
* @param[in] bulkTestCntrlStc         - pointer to a test running control structure
*                                       None
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddTrafficGenerate
(
    CPSS_IP_PROTOCOL_STACK_ENT                  ipvxProtVer,
    GT_U32                                      ipvxPrfxsQty,
    PRV_TGF_IPVX_LPM_PREFIXES_BULK_STORE_STC    *bulkTestCntrlStc
)
{
    GT_STATUS                       rc                  = GT_OK;
    GT_U32                          partsCount          = 0;
    GT_U32                          packetSize          = 0;
    TGF_PACKET_STC                  packetInfo          = {0};
    GT_U32                          portIter            = 0;
    GT_U32                          i                   = 0; /* iterator */
    GT_U32                          ii                  = 0;/* iterator */
    GT_U32                          numTriggers         = 0;
    GT_U32                          cpuTrigExpected     = 0;
    TGF_VFD_INFO_STC                vfdArray[1]         = {{0}};
    CPSS_INTERFACE_INFO_STC         portInterface       = {0};
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs           = {{{0}}};
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs       = {{{0}}};
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters          = {0};
    TGF_MAC_ADDR                    arpMacAddr          = {0};
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode         =
                                    PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E;

    /* number of VFDs in vfdArray */
    GT_U32      numVfd = 0;

    /* check if proper protocol and prefixesQty are entered */
    rc = prvTgfIpLpmIpvxUcPrefixBulkCheckParams(ipvxProtVer);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Bad input IP protocol type: %d",
                                                                   ipvxProtVer);

    /* copy default arp mac address base*/
    cpssOsMemCpy(&arpMacAddr, &(prvTgfArpMac), sizeof(TGF_MAC_ADDR));

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* AUTODOC: Iterate traffic generation prvTgfIpLpmBulkPrefixesQty times */
    for (i=0; i < ipvxPrfxsQty; i++)
    {
        /* ---------------------------------------------------------------------
         * 1. Setup counters and enable capturing
         */

        /* get routing mode */
        rc = prvTgfIpRoutingModeGet(&routingMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d",
                                                 prvTgfDevNum);

        /* reset Ethernet and IP counters for every prvTgfPortsArray port */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            /* reset Ethernet counters */
            rc = prvTgfResetCountersEth(prvTgfDevNum,
                                        prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                        "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum,
                                         prvTgfPortsArray[portIter]);

            if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
            {
                /* reset IP counters and set ROUTE_ENTRY mode */
                rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                            "prvTgfCountersIpSet: %d, %d",
                                             prvTgfDevNum,
                                             prvTgfPortsArray[portIter]);
            }
        }

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        /* AUTODOC: shift nexthop ports in circular mode according to        */
        /*          previously generated route entries (also in circular)    */
        portInterface.devPort.portNum = prvTgfPortsArray[((i%3) + 1)];

        /* enable capture on next hop port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                   TGF_CAPTURE_MODE_MIRRORING_E,
                                                   GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                   prvTgfDevNum, portInterface.devPort.portNum);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"tgfTrafficTableRxPcktTblClear");

        /* ---------------------------------------------------------------------
         * 2. Generating Traffic
         */

        /* calculate number of parts in packet */
        switch(ipvxProtVer)
        {
            case CPSS_IP_PROTOCOL_IPV4_E:
                partsCount = sizeof(prvTgfDefPacketIpv4) /
                             sizeof(prvTgfDefPacketIpv4[0]);
                /* calculate packet size */
                rc = prvTgfPacketSizeGet(prvTgfDefPacketIpv4, partsCount,
                                                            &packetSize);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

                /* copy next dest IP from previously generated bulk */
                cpssOsMemCpy(prvTgfDefHeaderIpv4.dstAddr,
                          &(bulkTestCntrlStc->ipv4Addr[i].arIP),
                            sizeof(TGF_IPV4_ADDR));
                packetInfo.partsArray = prvTgfDefPacketIpv4;

                break;
            case CPSS_IP_PROTOCOL_IPV6_E:
                partsCount = sizeof(prvTgfDefPacketIpv6) /
                             sizeof(prvTgfDefPacketIpv6[0]);
                /* calculate packet size */
                rc = prvTgfPacketSizeGet(prvTgfDefPacketIpv6, partsCount,
                                                            &packetSize);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

                /* copy next dest IP from previously generated bulk */
                for (ii = 0; ii < 8; ii++)
                {
                    prvTgfDefHeaderIpv6.dstAddr[ii] =
                           (GT_U16)bulkTestCntrlStc->ipv6Addr[i].arIP[ii*2 + 1];

                    prvTgfDefHeaderIpv6.dstAddr[ii] |=
                          ((GT_U16)bulkTestCntrlStc->ipv6Addr[i].arIP[ii*2])<<8;
                }
                packetInfo.partsArray = prvTgfDefPacketIpv6;

                break;
            default: break;
        }

        /* build packet */
        packetInfo.totalLen   = packetSize;
        packetInfo.numOfParts = partsCount;

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount,
                                 numVfd, &vfdArray[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                    "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* set transmit timings */
        rc = prvTgfSetTxSetup2Eth(0, 50);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                    "ERROR of prvTgfSetTxSetup2Eth: %d",
                                     prvTgfDevNum);

        prvTgfIpLpmBulkTestsCntr++;
        PRV_UTF_LOG1_MAC(" RUNNING TEST N%d \n", prvTgfIpLpmBulkTestsCntr);

        /* send packet - send from specific port - even if member of a trunk */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_DEF_SEND_PORT_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                    "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, PRV_TGF_DEF_SEND_PORT_CNS);
        cpssOsTimerWkAfter(1000);

        /* ---------------------------------------------------------------------
         * 3. Get Ethernet Counters
         */

        /* read and check Ethernet counters */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            GT_BOOL isOk;

            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                           prvTgfPortsArray[portIter],
                                           GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                        "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum,
                                         prvTgfPortsArray[portIter]);

            /* AUTODOC: verify routed packet on port 3 */
            if((prvTgfPortsArray[portIter] == PRV_TGF_DEF_SEND_PORT_CNS) ||
               ((prvTgfPortsArray[portIter] == portInterface.devPort.portNum) &&
                (i < bulkTestCntrlStc->ipvxAPQty)))
            {

                /* packetSize is not changed */
                    expectedCntrs.goodOctetsSent.l[0] =
                              (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  =
                              (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
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

            UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE,
                                        "get another counters values.");

            /* print expected values if bug */
            if (isOk != GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("Expected values:\n");
                PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n",
                                            expectedCntrs.goodOctetsSent.l[0]);
                PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n",
                                            expectedCntrs.goodPktsSent.l[0]);
                PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n",
                                            expectedCntrs.ucPktsSent.l[0]);
                PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n",
                                            expectedCntrs.brdcPktsSent.l[0]);
                PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n",
                                            expectedCntrs.mcPktsSent.l[0]);
                PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n",
                                            expectedCntrs.goodOctetsRcv.l[0]);
                PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n",
                                            expectedCntrs.goodPktsRcv.l[0]);
                PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n",
                                            expectedCntrs.ucPktsRcv.l[0]);
                PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n",
                                            expectedCntrs.brdcPktsRcv.l[0]);
                PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n",
                                            expectedCntrs.mcPktsRcv.l[0]);
                PRV_UTF_LOG0_MAC("\n");
            }
        }

        /* ---------------------------------------------------------------------
         * 4. Get Trigger Counters
         */

        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode          = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset        = 0;
        vfdArray[0].cycleCount    = sizeof(TGF_MAC_ADDR);

        /* expect for every new IP packet MAC address [prvTgfArpMac++] */
        arpMacAddr[5] = (GT_U8)i;
        cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));

        /* disable capture on nexthope port, before check packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                   TGF_CAPTURE_MODE_MIRRORING_E,
                                                   GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                   prvTgfDevNum, portInterface.devPort.portNum);

        /* check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1,
                                                        vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC(" numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum,
                                     portInterface.devPort.portNum);

        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        /* number of triggers should be according to number of transmit*/

        /* if we expect a packet*/
        if (i < bulkTestCntrlStc->ipvxAPQty)
        {
            cpuTrigExpected = ((1<<prvTgfBurstCount)-1);
        }
        else /* if not */
        {
            cpuTrigExpected = 0;
        }

        UTF_VERIFY_EQUAL6_STRING_MAC(cpuTrigExpected, numTriggers,
          "\n MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

        /* ---------------------------------------------------------------------
         * 5. Get IP Counters
         */

        /* AUTODOC: get and print ip counters values */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
            PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n",
                              prvTgfPortsArray[portIter]);
            prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
        }
        PRV_UTF_LOG0_MAC("\n");
    }
    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d",
                                                                 prvTgfDevNum);

    /* disable the PCL exclude vid capturing */
    rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_FALSE,0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
      "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);
}

/**
* @internal prvTgfIpLpmIpUcPrefixBulkAddConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore Route Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfIpLpmIpUcPrefixBulkAddConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      portIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS   rc        = GT_OK;
    PRV_TGF_MAC_ENTRY_KEY_STC  mcEntryDelKey = {0};

    /* prepare mac entry to be deleted from FDB */
    cpssOsMemSet(&mcEntryDelKey, 0, sizeof(mcEntryDelKey));
    cpssOsMemCpy(mcEntryDelKey.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));
    mcEntryDelKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    mcEntryDelKey.key.macVlan.vlanId = PRV_TGF_SEND_VLANID_CNS;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* clear bulk tests counter*/
    prvTgfIpLpmBulkTestsCntr = 0;

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgFdbFlush: %d",
                                 prvTgfDevNum);

    rc = prvTgfBrgFdbMacEntryDelete(&mcEntryDelKey);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    /* AUTODOC: disable Unicast IPv4/Ipv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_DEF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, 0);

    /* AUTODOC: disable IPv4/Ipv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4V6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* restore port inlif mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_DEF_SEND_PORT_IDX_CNS,
                                prvTgfRestoreCfg.sendPortInlifMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc,prvTgfInlifPortModeSet);


    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* delete port from both VLANs */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* delete Ports from VLANs */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum,
                                           prvTgfVlanArray[vlanIter],
                                           prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                       "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                        prvTgfDevNum, prvTgfVlanArray[vlanIter],
                                        prvTgfPortsArray[portIter]);
        }

        /* delete VLANs */

        /* AUTODOC: invalidate vlan entries 5,6
         */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                    "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

#ifdef EXMXPM_FAMILY
    /* for puma devices restore inlif parameters */
    rc = cpssExMxPmInlifEntrySet(0, PRV_TGF_INLIF_TYPE_PORT_E,
                                    PRV_TGF_DEF_SEND_PORT_IDX_CNS,
                                   &(prvTgfRestoreCfg.inlifSave));
#endif

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d",
                                                                 prvTgfDevNum);
}


