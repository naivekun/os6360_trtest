/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfIpv4Uc2MultipleVr.c
*
* @brief Create 1959 virtual routers. Add one prefix matching route per VR.
* A regular route of every VR routes packets with IP DA = 1.1.1.3 into
* vlan 3 (TX_VLAN_CNS). A default route of VR routes packets into vlan 4
* (DEFAULT_TX_VLAN_CNS). A packet MAC DA is set to 00:00:00:0f:xx:yy,
* where xx= vrId/100, yy = vrId%100 (i.e. MAC DA is unique for VR).
*
* To generate traffic we:
* - place an rx port (index 0) into 24 vlans (VLANS_COUNT_CNS).
* - place a tx port (index 1) into vlans 3,4 (TX_VLAN_CNS,
* DEFAULT_TX_VLAN_CNS).
* - in cycle through all vrId with step=24 assign unique vrId to
* every of 24 vlan.
* - generate traffic with burst = 24 twice(vlan id will be changed
* incrementally in every packet): with IP = 1.1.1.3 (to be routed by
* a regular route) and with IP 1.1.15.15 (routed by a default route).
* - check tx port counters and MAC DA, vlanId of captured packets.
* Test is not applicable for EXMXPM devices.
*
* @version   2
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
#include <ip/prvTgfIpv4Uc2MultipleVr.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* first index of new entry in the Next-Hop table */
#define PRV_TGF_FIRST_ROUTE_INDEX_CNS 1

/* first value of vrId */
#define PRV_TGF_FIRST_VR_ID_CNS 1

/* maximum vrId (can't exceed PRV_CPSS_DXCH_LPM_RAM_NUM_OF_VIRTUAL_ROUTERS_CNS).
   Note: It's set to 1536 because of memory issues (sip5 devices) and because
   of ipTcamShadowPtr->numOfIpPrefixes[CPSS_IP_PROTOCOL_IPV4_E] = 3920 (DXCH).
*/
#define PRV_TGF_LAST_VR_ID_CNS 1248

/* total number of created virtual routers */
#define PRV_TGF_VR_COUNT_CNS ((PRV_TGF_LAST_VR_ID_CNS) - \
                              (PRV_TGF_FIRST_VR_ID_CNS) + 1)

/* vlan number It shouldn't exceed 0xFF */
#define PRV_TGF_FIRST_VLAN_CNS 3


/* total number of VLANs. At least 2 VLANs are required to distinguish packets
   sent by a default route and sent by a matching prefix route. Max vlan id
   shouldn't exceed 0xFF */
#define PRV_TGF_VLANS_COUNT_CNS 24

/* MAC DA offset in the packet buffer */
#define PRV_TGF_MAC_DA_OFFSET_CNS  0

/* offset of low byte of vlanId in the packet buffer. */
#define PRV_TGF_VLANID_LOW_BYTE_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS + \
                                             TGF_VLAN_TAG_SIZE_CNS - 1)

/* vlan used to send packets by an matching prefix route. */
#define PRV_TGF_TX_VLAN_CNS  PRV_TGF_FIRST_VLAN_CNS

/* vlan for default routes */
#define PRV_TGF_DEFAULT_TX_VLAN_CNS  PRV_TGF_FIRST_VLAN_CNS+1

/* port index where a traffic will come in */
#define PRV_TGF_RX_PORT_IDX_CNS  0

/* port index where VR will send a traffic */
#define PRV_TGF_TX_PORT_IDX_CNS 1

/* destination IP address to be routed by a prefix matching route */
static TGF_IPV4_ADDR  prvTgfDstIp = {1, 1, 1, 3};

/* destination IP address to be routed by a default route */
static TGF_IPV4_ADDR  prvTgfDefaultDstIp = {1, 1, 15, 15};

/* the LPM DB id for LPM Table */
static GT_U32 prvTgfLpmDBId      = 0;

/******************************* Test packet **********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_FIRST_VLAN_CNS                        /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
                                     {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
                    4,                                  /* version */
                    5,                                  /* headerLen */
                    0,                                  /* typeOfService */
                    0x2A,                               /* totalLen */
                    0,                                  /* id */
                    0,                                  /* flags */
                    0,                                  /* offset */
                    0x40,                               /* timeToLive */
                    0xFF,                               /* protocol */
                    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
                    { 2,  2,  2,  2},                   /* srcAddr */
                    { 1,  1,  1,  3}                    /* dstAddr */
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

/* LENGTH of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)


/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfIpv4Uc2MultipleVrCreateVlans function
* @endinternal
*
* @brief   Create all vlans, used by the test.
*
* @retval GT_OK                    - on success
*/

static GT_STATUS prvTgfIpv4Uc2MultipleVrCreateVlans
(
    GT_VOID
)
{
    CPSS_PORTS_BMP_STC                 portsMembers;
    CPSS_PORTS_BMP_STC                 portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC          vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    GT_STATUS                          rc;
    GT_U16                             i;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));

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
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.vrfId = 0;   /* this field will be initialized later
                           in the traffic generating function */

    for (i = 0; i < PRV_TGF_VLANS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, PRV_TGF_FIRST_VLAN_CNS + i,
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("\n prvTgfBrgVlanEntryWrite, vlanId=%d, rc=%d\n",
                             PRV_TGF_FIRST_VLAN_CNS + i, rc);
            return rc;
        };
    };
    return GT_OK;
};


/**
* @internal prvTgfIpv4Uc2MultipleVrInitVlansVrId function
* @endinternal
*
* @brief   Assign vlans vrId value.
*
* @param[in] initialVrId              - vrId will be assign in order starting from this one.
*                                      count       - number of VLANs to be assigned.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfIpv4Uc2MultipleVrInitVlansVrId
(
    IN GT_U32 initialVrId,
    IN GT_U32 vlanCount
)
{
    GT_U16    i;
    GT_STATUS rc;

    if ( vlanCount > PRV_TGF_VLANS_COUNT_CNS ||
         initialVrId + vlanCount - 1 > PRV_TGF_LAST_VR_ID_CNS)
    {
        return GT_BAD_PARAM;
    };

    for (i = 0; i < PRV_TGF_VLANS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_FIRST_VLAN_CNS + i, initialVrId + i);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("prvTgfBrgVlanVrfIdSet error, vlan=%d, vrId=%d",
                             PRV_TGF_FIRST_VLAN_CNS + i, initialVrId + i);
            return rc;
        }
    };
    return GT_OK;
};

/**
* @internal prvTgfIpv4Uc2MultipleVrCalcMacAddrPerVrId function
* @endinternal
*
* @brief   Generate macAddr by virtual router id.
*
* @param[in] vrId                     - Virtual Router Id
*
* @param[out] macAddr                  - destination MAC address.
*                                       None
*/
static GT_VOID prvTgfIpv4Uc2MultipleVrCalcMacAddrPerVrId
(
    IN   GT_U32       vrId,
    OUT  TGF_MAC_ADDR macAddr
)
{
    cpssOsMemSet(macAddr, 0, sizeof(TGF_MAC_ADDR));
    macAddr[3] = 0x0F;
    macAddr[4] = (GT_U8)(vrId / 100);
    macAddr[5] = (GT_U8)(vrId % 100);
}


/**
* @internal prvTgfIpv4Uc2MultipleVrCreateVirtualRouters function
* @endinternal
*
* @brief   - Create PRV_TGF_VR_COUNT_CNS entries in the Next-Hop Table
*         - Create an entry in the Next-Hop Table that will be used as default
*         for newly created virtual routers.
*         - Create PRV_TGF_VR_COUNT_CNS virtual routers
*         - Add one prefix for every VR into LPM
*         - Add an entry in the Next-Hop Table for every prefix. This
*         entries will route packet to the tx port with unique MAC DA.
* @param[in] createAllVrBeforeAddingPrefixes - whether to add the prefixes only after
*                                      all the virtual routers are created
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfIpv4Uc2MultipleVrCreateVirtualRouters
(
    GT_BOOL createAllVrBeforeAddingPrefixes
)
{
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           ucRouteEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    routeEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defUcRouteEntryInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  vrIter              = 0;
    GT_U32                                  arpIndex            = 0;
    GT_U32                                  vrId                = 0;
    GT_U32                                  routeIndex          = 0;
    GT_U32                                  defRouteIndex       = 0;
    GT_STATUS                               rc                  = GT_FAIL;
    const GT_U32                            numOfPaths          = 0;

    cpssOsMemSet(&arpMacAddr, 0, sizeof(arpMacAddr));
    cpssOsMemSet(&ucRouteEntry, 0, sizeof(ucRouteEntry));
    cpssOsMemSet(&routeEntryInfo, 0, sizeof(routeEntryInfo));
    cpssOsMemSet(&defUcRouteEntryInfo, 0, sizeof(defUcRouteEntryInfo));
    cpssOsMemSet(&ipAddr, 0, sizeof(ipAddr));

    /* AUTODOC: Create a new Virtual Router for the LPM DB */

    /*  Create all virtual routers and route prefixes in the LPM DB  */
    for (vrIter = 0; vrIter < PRV_TGF_VR_COUNT_CNS; vrIter++)
    {
        /* ARP indexes in ARP table starting from 1 */
        arpIndex = 1 + vrIter;

        /* vrId of new virtual router */
        vrId = PRV_TGF_FIRST_VR_ID_CNS + vrIter;

        /* on every iteration we write two routes in the Next Hop Table:
           default route and regular prefix matching route. */
        routeIndex    = PRV_TGF_FIRST_ROUTE_INDEX_CNS + 2 * vrIter;
        defRouteIndex = routeIndex + 1;

        PRV_UTF_LOG1_MAC("\n Create Virtual Router and Route Entries, vrId=%d\n",
                         vrId);

        /* AUTODOC: Create two route entries in the Route Table(Next Hop Table).
              for every VR: a default route and a prefix matching route.
              This entries distinguish by a  nextHop vlan id only.            */

        /* write an ARP MAC address into the Router ARP Table (one per vrId). */
        prvTgfIpv4Uc2MultipleVrCalcMacAddrPerVrId(vrId, arpMacAddr.arEther);

        /* AUTODOC: write ARP macAddr as 00:00:00:0f:xx:yy,
             where xx= vrId/100, yy = vrId%100                              */
        rc = prvTgfIpRouterArpAddrWrite(arpIndex, &arpMacAddr);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpRouterArpAddrWrite");

        /* create a UC Route entry with CMD_ROUTE */
        ucRouteEntry.cmd                               = CPSS_PACKET_CMD_ROUTE_E;
        ucRouteEntry.nextHopInterface.type             = CPSS_INTERFACE_PORT_E;
        ucRouteEntry.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
        ucRouteEntry.nextHopInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
        ucRouteEntry.nextHopARPPointer                 = arpIndex;

        /* write a default route */
        ucRouteEntry.nextHopVlanId                    = PRV_TGF_DEFAULT_TX_VLAN_CNS;
        rc = prvTgfIpUcRouteEntriesWrite(defRouteIndex, &ucRouteEntry, 1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpUcRouteEntriesWrite");

        /* write a regular route */
        ucRouteEntry.nextHopVlanId                    = PRV_TGF_TX_VLAN_CNS;
        rc = prvTgfIpUcRouteEntriesWrite(routeIndex, &ucRouteEntry, 1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpUcRouteEntriesWrite");

        PRV_UTF_LOG3_MAC("nextHopVlanId = %d, portNum = %d, ARPPtr = %d\n",
                         ucRouteEntry.nextHopVlanId,
                         ucRouteEntry.nextHopInterface.devPort.portNum,
                         ucRouteEntry.nextHopARPPointer);


        /* set defUcRouteEntryInfo */
        defUcRouteEntryInfo.ipLttEntry.routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
        defUcRouteEntryInfo.ipLttEntry.numOfPaths               = numOfPaths;
        defUcRouteEntryInfo.ipLttEntry.routeEntryBaseIndex      = defRouteIndex;
        defUcRouteEntryInfo.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;
        defUcRouteEntryInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        defUcRouteEntryInfo.ipLttEntry.ipv6MCGroupScopeLevel    = 0;

        /* AUTODOC: create Virtual Routers with default route entries. */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, vrId,
                                         &defUcRouteEntryInfo,
                                         NULL, NULL, NULL, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLpmVirtualRouterAdd");

        /* AUTODOC: create prefixes in the LPM Table */

        /* add LPM prefix with DIP = 1.1.1.3 for every regular route*/
        cpssOsMemCpy(ipAddr.arIP, prvTgfDstIp, sizeof(ipAddr.arIP));

        if (createAllVrBeforeAddingPrefixes == GT_FALSE)
        {
            /* set routeEntryBaseIndex to the regular route index */
            routeEntryInfo.ipLttEntry.routeEntryBaseIndex = routeIndex;
            routeEntryInfo.ipLttEntry.numOfPaths          = numOfPaths;
            routeEntryInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

            /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId,
                                            ipAddr, 32, &routeEntryInfo, GT_FALSE,GT_FALSE);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLpmIpv4UcPrefixAdd");
        }
    }
    if (createAllVrBeforeAddingPrefixes == GT_TRUE)
    {
        for (vrIter = 0; vrIter < PRV_TGF_VR_COUNT_CNS; vrIter++)
        {
            /* vrId of new virtual router */
            vrId = PRV_TGF_FIRST_VR_ID_CNS + vrIter;
            routeIndex    = PRV_TGF_FIRST_ROUTE_INDEX_CNS + 2 * vrIter;

            /* set routeEntryBaseIndex to the regular route index */
            routeEntryInfo.ipLttEntry.routeEntryBaseIndex = routeIndex;
            routeEntryInfo.ipLttEntry.numOfPaths          = numOfPaths;
            routeEntryInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

            /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, vrId,
                                            ipAddr, 32, &routeEntryInfo, GT_FALSE,GT_FALSE);
            PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpLpmIpv4UcPrefixAdd");
        }
    }
    return GT_OK;
};

/**
* @internal prvTgfIpv4Uc2MultipleVrSendBurst function
* @endinternal
*
* @brief   Send burstCount packets through one call of prvTgfStartTransmitingEth.
*         Every packet will be sent in a different vlan. Every vlan will be
*         assigned with unique vrId.
* @param[in] initialVrId              - initial vrId number
* @param[in] burstCount               - packets count in the burst.
* @param[in] isDef                    - set packet IP DA to be routed by default route (GT_TRUE)
*                                      or to be routed by regular route (GT_FALSE).
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfIpv4Uc2MultipleVrSendBurst
(
    IN GT_U32  initialVrId,
    IN GT_U32  burstCount,
    IN GT_BOOL isDef
)
{
    TGF_PACKET_STC               packetInfo;
    TGF_VFD_INFO_STC             vfdArray[1];

    GT_U32                       partsCount        = 0;
    GT_STATUS                    rc                = GT_FAIL;


    cpssOsMemSet(&packetInfo,    0, sizeof(packetInfo));
    cpssOsMemSet(vfdArray,       0, sizeof(vfdArray));

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) /
                 sizeof(prvTgfPacketPartArray[0]);

    packetInfo.totalLen   = PRV_TGF_IPV4_PACKET_LEN_CNS;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* assign vrId to vlans */
    rc = prvTgfIpv4Uc2MultipleVrInitVlansVrId(initialVrId, burstCount);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv4Uc2MultipleVrInitVlansVrId");

    /* fill destination IP address for the packet */
    cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr,
                 ((GT_TRUE == isDef) ? prvTgfDefaultDstIp : prvTgfDstIp),
                 sizeof(prvTgfPacketIpv4Part.dstAddr));

    /* configure VFD to increment vlan tag. We assume that a tag doesn't
       exceed 0xFF! So it is enough to modify only low byte of tag value. */
    vfdArray[0].modeExtraInfo   = 0;
    vfdArray[0].mode            = TGF_VFD_MODE_INCREMENT_E;
    vfdArray[0].offset          = PRV_TGF_VLANID_LOW_BYTE_OFFSET_CNS;
    vfdArray[0].cycleCount      = 1;
    vfdArray[0].patternPtr[0]   = PRV_TGF_FIRST_VLAN_CNS;

    /* setup the packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo,
                             burstCount, 1, vfdArray);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* send burstCount number of packets */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("prvTgfStartTransmitingEth error, dev=%d, port=%d",
                         prvTgfDevNum,
                         prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
        return rc;
    };

    return GT_OK;

};

/**
* @internal prvTgfIpv4Uc2MultipleVrCheckCounters function
* @endinternal
*
* @brief   Check port counters and a MAC DA, vlanId fields of captured packets.
*         It is expected that a packets are sent through the same burst. It will
*         be checked what a packets was routed by Virtual Routers with vrId
*         starting from initialVrId (one packet per VR).
* @param[in] initialVrId              - initial vrId value.
* @param[in] burstCount               - number of captured packets.
* @param[in] isDef                    - packets should be routed by a default route of VR.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfIpv4Uc2MultipleVrCheckCounters
(
    IN GT_U32  initialVrId,
    IN GT_U32  burstCount,
    IN GT_BOOL isDef
)
{
    TGF_MAC_ADDR            expMacDa;
    TGF_NET_DSA_STC         rxParam;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U8                   packetBuf[PRV_TGF_IPV4_PACKET_LEN_CNS];
    GT_U32                  portIter         = 0;
    GT_U8                   devNum           = 0;
    GT_U8                   queue            = 0;
    GT_U32                  packetLen        = 0;
    GT_U32                  origPacketLen    = 0;
    GT_U32                  vrId             = initialVrId;
    GT_U8                   expVlanId        = 0;   /* a low byte of exprected vlanId */
    GT_U32                  expRxPacketCount = 0;
    GT_U32                  expTxPacketCount = 0;
    GT_U32                  packetsCount     = 0;
    GT_BOOL                 isFirst          = GT_TRUE;
    GT_STATUS               rc               = GT_OK;


    if (initialVrId + burstCount - 1 > PRV_TGF_LAST_VR_ID_CNS)
    {
        return GT_BAD_PARAM;
    };

    cpssOsMemSet(&expMacDa, 0, sizeof(expMacDa));
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));
    cpssOsMemSet(packetBuf, 0, sizeof(packetBuf));
    packetLen = PRV_TGF_IPV4_PACKET_LEN_CNS;

    /* check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* calculate expected counters */
        if (PRV_TGF_RX_PORT_IDX_CNS == portIter ||
            PRV_TGF_TX_PORT_IDX_CNS == portIter)
        {
            expRxPacketCount = 1;
            expTxPacketCount = 1;
        }
        else
        {
            expRxPacketCount = 0;
            expTxPacketCount = 0;
        };

        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter],
                                    expRxPacketCount, expTxPacketCount,
                                    packetLen, burstCount);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG3_MAC("\n prvTgfEthCountersCheck port=%d,"\
                             "expected: Rx=%d, Tx=%d\n",
                             prvTgfPortsArray[portIter],
                             expRxPacketCount * burstCount,
                             expTxPacketCount * burstCount);
            return rc;
        };
    }


    /* check MAC DA and vlan id of captured packets */

    /* setup nextHop portInterface */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    expVlanId = (isDef == GT_TRUE) ? PRV_TGF_DEFAULT_TX_VLAN_CNS :
                                     PRV_TGF_TX_VLAN_CNS;

    /* get first or next captured packet from expected interface */
    isFirst = GT_TRUE;
    /* iterate through all captured packets */
    vrId=initialVrId;
    packetsCount = 0;
    do
    {
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(
                                                   &portInterface,
                                                   TGF_PACKET_TYPE_CAPTURE_E,
                                                   isFirst, GT_FALSE, packetBuf,
                                                   &packetLen, &origPacketLen,
                                                   &devNum, &queue, &rxParam);
        isFirst = GT_FALSE;  /* now we get the next */
        if (rc != GT_OK)
        {
            break;
        };
        packetsCount++;

        /* compare vlanId (12 bits). */
        if (expVlanId != packetBuf[PRV_TGF_VLANID_LOW_BYTE_OFFSET_CNS] ||
            0 != (packetBuf[PRV_TGF_VLANID_LOW_BYTE_OFFSET_CNS-1] & 0x0F))
        {
            PRV_UTF_LOG2_MAC("Unexpected vlanId. Exprected %d, received %d",
                             expVlanId,
                             packetBuf[PRV_TGF_VLANID_LOW_BYTE_OFFSET_CNS]);
            return GT_BAD_VALUE;
        };

        /* compare MAC DA */
        prvTgfIpv4Uc2MultipleVrCalcMacAddrPerVrId(vrId, expMacDa);
        if (cpssOsMemCmp(&expMacDa, &packetBuf[PRV_TGF_MAC_DA_OFFSET_CNS],
                         sizeof(TGF_MAC_ADDR)) != 0)
        {
            PRV_UTF_LOG0_MAC("Unexpected MAC DA\n");
            PRV_UTF_LOG6_MAC("Received: %02X:%02X:0%2X:%02X:%02X:%02X\n",
                             packetBuf[0], packetBuf[1], packetBuf[2],
                             packetBuf[3], packetBuf[4], packetBuf[5]);

            PRV_UTF_LOG6_MAC("Expected: %02X:%02X:0%2X:%02X:%02X:%02X\n",
                             expMacDa[0], expMacDa[1], expMacDa[2],
                             expMacDa[3], expMacDa[4], expMacDa[5]);
            return GT_BAD_VALUE;
        };
        vrId++;
    }
    while (GT_TRUE);

    rc = (GT_NO_MORE == rc ? GT_OK : rc);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv4Uc2MultipleVrCheckCountersAfterBurst");

    if (packetsCount != burstCount)
    {
        PRV_UTF_LOG2_MAC("Number of captured packets is wrong." \
                         "Expected %d, received %d",
                         burstCount,
                         packetsCount);
        return GT_FAIL;
    }

    return GT_OK;
};



/************************** PUBLIC DECLARATIONS *******************************/

/**
* @internal prvTgfIpv4Uc2MultipleVrBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv4Uc2MultipleVrBaseConfigurationSet
(
    GT_VOID
)
{
    GT_U32                              port;
    GT_U16                             vlanId           = 0;
    GT_STATUS                          rc               = GT_FAIL;

    /* AUTODOC: SETUP BASE CONFIGURATION:                    */
    /* AUTODOC: - create vlans                               */
    /* AUTODOC: - add rx port to all vlans (tagged)          */
    /* AUTODOC: - add tx ports to vlans (tagged)             */
    /* AUTODOC:      PRV_TGF_TX_VLAN_CNS,                    */
    /* AUTODOC:      PRV_TGF_DEFAULT_TX_VLAN_CNS.            */
    /* AUTODOC: - add tx ports to vlans PRV_TGF_TX_VLAN_CNS, */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    /******************* create VLANs ************************/
    rc = prvTgfIpv4Uc2MultipleVrCreateVlans();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpv4Uc2MultipleVrCreateVlans");

    /* AUTODOC: Add rx port to all VLANs.                 */
    /* AUTODOC: add tx port to vlans PRV_TGF_TX_VLAN_CNS, */
    /* AUTODOC:              PRV_TGF_DEFAULT_TX_VLAN_CNS. */

    /******************* move ports to vlans ************************/
    /* add rx port to all vlans (tagged) */
    port = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    for (vlanId = PRV_TGF_FIRST_VLAN_CNS;
         vlanId < PRV_TGF_FIRST_VLAN_CNS + PRV_TGF_VLANS_COUNT_CNS;
         vlanId++)
    {
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, vlanId, port, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfBrgVlanMemberAdd: %d, %d, %d",
                                     prvTgfDevNum, vlanId, port);
    };

    /* add tx port to vlans (tagged) PRV_TGF_TX_VLAN_CNS,
                                     PRV_TGF_DEFAULT_TX_VLAN_CNS */
    port = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    vlanId = PRV_TGF_TX_VLAN_CNS;
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, vlanId, port, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanMemberAdd: %d, %d, %d",
                                 prvTgfDevNum, vlanId, port);

    vlanId = PRV_TGF_DEFAULT_TX_VLAN_CNS;
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, vlanId, port, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanMemberAdd: %d, %d, %d",
                                 prvTgfDevNum, vlanId, port);
};

/**
* @internal prvTgfIpv4Uc2MultipleVrRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] createAllVrBeforeAddingPrefixes - whether to add the prefixes only after
*                                      all the virtual routers are created
*                                       None
*/
GT_VOID prvTgfIpv4Uc2MultipleVrRouteConfigurationSet
(
    GT_BOOL createAllVrBeforeAddingPrefixes
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    GT_U16                      vlanIter;
    GT_STATUS                   rc          = GT_FAIL;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Route Configuration =======\n");

    /* AUTODOC: for every rx port(0) vlan place MAC 00:00:00:00:34:02 into FDB
        and mark it as Router MAC address (macEntry.daRoute = GT_TRUE)    */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac,  sizeof(TGF_MAC_ADDR));
    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = 0; /* will be changed */
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
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

    for (vlanIter = PRV_TGF_FIRST_VLAN_CNS;
         vlanIter < PRV_TGF_FIRST_VLAN_CNS + PRV_TGF_VLANS_COUNT_CNS;
         vlanIter++)
    {
        macEntry.key.key.macVlan.vlanId = vlanIter;
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite");

        /* AUTODOC: enable Unicast IPv4 Routing on all vlans. */
        rc = prvTgfIpVlanRoutingEnable(vlanIter, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d",
                                     vlanIter);

    };

    /* AUTODOC: enable Unicast IPv4 Routing on rx port. */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_RX_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpPortRoutingEnable: %d",
                                 prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

   rc = prvTgfIpv4Uc2MultipleVrCreateVirtualRouters(createAllVrBeforeAddingPrefixes);
   UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "prvTgfIpv4Uc2MultipleVrCreateVirtualRouters");
}

/**
* @internal prvTgfIpv4Uc2MultipleVrTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpv4Uc2MultipleVrTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;

    GT_U32                  i;
    GT_U32                  portIter;
    GT_U32                  initialVrId;
    GT_BOOL                 isDef            = GT_TRUE;
    GT_U32                  packetLen        = 0;
    GT_U32                  burstCount       = PRV_TGF_VLANS_COUNT_CNS;
    GT_STATUS               rc               = GT_FAIL;

    /* AUTODOC: GENERATE TRAFFIC: */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* calculate packet length */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray,
                             sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]),
                             &packetLen);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet");

    /* setup nextHop portInterface for capturing */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* AUTODOC: Iterate through all VR and send for every 24 VR two bursts of */
    /* AUTODOC: packets: 1st - to be routed by a reqular route by VRs,        */
    /* AUTODOC: 2nd - to be routed by a default route of VR                   */
    burstCount = PRV_TGF_VLANS_COUNT_CNS;

    for (initialVrId  = PRV_TGF_FIRST_VR_ID_CNS;
         initialVrId <= PRV_TGF_LAST_VR_ID_CNS;
         initialVrId += burstCount)
    {

        /* check if a burstCount is too big (it's possible on last iteration) */
        if (burstCount > PRV_TGF_LAST_VR_ID_CNS - initialVrId  + 1)
        {

            burstCount = PRV_TGF_LAST_VR_ID_CNS - initialVrId + 1;
        };

        /* 1st iteration - packets will be routed by regular route,
           2nd iteration - packets will be routed by default route. */
        for (i=0; i<2; i++)
        {
            isDef = (0 == i) ? GT_FALSE : GT_TRUE;

            /* AUTODOC: Setup counters and enable capturing on tx port. */
            /* reset ethernet counters */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                rc = prvTgfResetCountersEth(prvTgfDevNum,
                                            prvTgfPortsArray[portIter]);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                             "prvTgfResetCountersEth: %d",
                                             prvTgfPortsArray[portIter]);
            };

            /* enable capturing on nextHope port */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                        TGF_CAPTURE_MODE_MIRRORING_E,
                                                        GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "tgfTrafficGeneratorPortTxEthCaptureSet: %d\n",
                                         portInterface.devPort.portNum);

            /* clear capturing RxPcktTable */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "tgfTrafficTableRxPcktTblClear");

            /* AUTODOC: Setup the packet and generate traffic to rx port 0.  */
            /* AUTODOC: DA=00:00:00:00:34:02, SA=00:00:00:00:00:22           */
            /* AUTODOC: VID=<FIRST_VLAN+i, i=0..VLANS_COUNT>, srcIP=1.1.1.1, */
            /* AUTODOC: dstIP=1.1.1.3 (routed by a prefix matching route)    */
            /* AUTODOC: or 1.1.15.15 (routed by a  default route).           */
            rc = prvTgfIpv4Uc2MultipleVrSendBurst(initialVrId, burstCount, isDef);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "prvTgfIpv4Uc2MultipleVrSendBurst");

            cpssOsTimerWkAfter(10);

            /* disable capturing on nextHop port */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                        TGF_CAPTURE_MODE_MIRRORING_E,
                                                        GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "%d \n",
                                         portInterface.devPort.portNum);

            /* AUTODOC: Check port ethernet counters and check MAC DA, vlanId */
            /* AUTODOC: values of captured packets. */

            rc = prvTgfIpv4Uc2MultipleVrCheckCounters(
                                                initialVrId, burstCount, isDef);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                         "prvTgfIpv4Uc2MultipleVrCheckCounters %d \n",
                         portInterface.devPort.portNum);
        };

    }
}


/**
* @internal prvTgfIpv4Uc2MultipleVrConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpv4Uc2MultipleVrConfigurationRestore
(
    GT_VOID
)
{
    GT_U32       port;
    GT_U32      vrId;
    GT_U16      vlanIter;
    GT_STATUS   rc        = GT_FAIL;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore Route Configuration  */
    for (vrId = PRV_TGF_FIRST_VR_ID_CNS; vrId <= PRV_TGF_LAST_VR_ID_CNS; vrId++)
    {

        /* AUTODOC: flush the UC IPv4 Routing table */
        rc = prvTgfIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, vrId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfIpLpmIpv4UcPrefixesFlush: %d", vrId);

        /* AUTODOC:  delete virtual routers */
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel");
    }

    /* AUTODOC:  Restore Base Configuration   */
    /* AUTODOC: delete ports from VLANs */

    /* delete tx port from vlans */
    port = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_DEFAULT_TX_VLAN_CNS,
                                   port);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEFAULT_TX_VLAN_CNS,
                                 port);

    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_TX_VLAN_CNS, port);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_TX_VLAN_CNS, port);

    /* delete rx port from all vlans*/
    port = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    for (vlanIter = 0; vlanIter < PRV_TGF_VLANS_COUNT_CNS; vlanIter++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum,
                                       PRV_TGF_FIRST_VLAN_CNS + vlanIter,
                                       port);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum,
                                     PRV_TGF_FIRST_VLAN_CNS + vlanIter,
                                     port);
    };

    /* AUTODOC: delete VLANs */
    for (vlanIter = 0; vlanIter < PRV_TGF_VLANS_COUNT_CNS; vlanIter++)
    {
        /* AUTODOC: disable IPv4 Unicast Routing on vlan */
        rc = prvTgfIpVlanRoutingEnable(PRV_TGF_FIRST_VLAN_CNS + vlanIter,
                                       CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpVlanRoutingEnable: %d",
                                     PRV_TGF_FIRST_VLAN_CNS + vlanIter);

        /*  AUTODOC: invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_FIRST_VLAN_CNS + vlanIter);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfBrgVlanEntryInvalidate: %d",
                                     PRV_TGF_FIRST_VLAN_CNS + vlanIter);
    }

    /* AUTODOC: disable Unicast IPv4 Routing on RX port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_RX_PORT_IDX_CNS,
                                   CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E,
                                   GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpPortRoutingEnable: %d",
                                 prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

}


