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
* @file prvTgfBasicIpv4UcEcmpRouting.c
*
* @brief Basic IPV4 UC ECMP Routing
*
* @version   23
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
#include <ip/prvTgfBasicIpv4UcEcmpRouting.h>



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

/* default number of packets to send */
static GT_U32        prvTgfBurstCount          = 1;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the Number of paths for the Router Table */
static GT_U32        prvTgfNumOfPaths          = 64;

/* Base Indexes for the Route, ARP, SIP and ChkSum */
static GT_U32        prvTgfRouteEntryBaseIndex = 60;
static GT_U32        prvTgfEcmpEntryBaseIndex  = 89;
static GT_U32        prvTgfArpEntryBaseIndex   = 0;
static GT_U8         prvTgfSipBaseIndex        = 1;
static GT_U8         prvTgfSipStep             = 1;

/* Restored values */
static GT_U32        nextHopIndexRestore[64];
static GT_BOOL       indirectAcessEnableRestore = GT_FALSE;

/********************* Router ARP Table Configuration *************************/

static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x11, 0x22, 0x00};

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

static PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT globalHashModeGet;
static PRV_TGF_TRUNK_LBH_CRC_MODE_ENT    crcHashModeGet;
static GT_U32                            crcSeedGet;
static GT_U32                            startBitGet;
static GT_U32                            numOfBitsGet;
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* create a static macEntry in SEND VLAN with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum =
                                    prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    macEntry.isStatic                     = GT_TRUE;
    macEntry.daRoute                      = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
           "prvTgfBrgFdbMacEntrySet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
static GT_VOID prvTgfBasicIpv4UcEcmpRoutingLttRouteConfigurationSet
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
     *   nextHopARPPointer   = prvTgfArpEntryBaseIndex   + pathIndex
     *   countSet            = pathIndex % 4
     * IP packets will use following rules:
     *   sip   = sipBase   + pathIndex;
     *   chsum = chsumBase - pathIndex;
     *   daMac = daMacBase + pathIndex;
     */
    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
    {
        /* write a ARP MAC address to the Router ARP Table */
        prvTgfArpMac[5] = (GT_U8) (prvTgfArpEntryBaseIndex + pathIndex);

        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        /* AUTODOC: write 8 ARP MACs 00:00:00:11:22:[01..08]  to the Router ARP Table */
        rc = prvTgfIpRouterArpAddrWrite(prvTgfArpEntryBaseIndex + pathIndex, &arpMacAddr);
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
        regularEntryPtr->nextHopARPPointer = prvTgfArpEntryBaseIndex + pathIndex;

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

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 and numOfPaths 7 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

static GT_U32   debug_EcmpRoutingHash_prvTgfNumOfPaths = 100;
/**
* @internal prvTgfBasicIpv4UcEcmpRoutingHashCheckLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT and Hash Route Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
static GT_VOID prvTgfBasicIpv4UcEcmpRoutingHashCheckLttRouteConfigurationSet
(
    GT_BOOL randomEnable
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U8                                   pathIndex;
    PRV_TGF_IP_ECMP_ENTRY_STC               ecmpEntry;
    GT_U32                                  nextHopPort;
    GT_U32                                  crcSeed = 0;
    GT_U32                                  i= 0;
    GT_U8                                   saltValue = 0;
    PRV_TGF_PCL_PACKET_TYPE_ENT             packetType = PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E;
    PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC    maskEntry = {0};

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");
    cpssOsMemSet(&maskEntry, 0, sizeof(maskEntry));

    /* define max number of paths - up to 4K Route entries in a single ECMP block */
    prvTgfNumOfPaths = 100;
    if(GT_TRUE == prvUtfIsGmCompilation() || prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E))
    {
        prvTgfNumOfPaths = debug_EcmpRoutingHash_prvTgfNumOfPaths;
    }

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

    /* Get the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    rc =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

    /* Set the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    rc =  prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* get crc hash parameters */
    rc =  prvTgfTrunkHashCrcParametersGet(prvTgfDevNum, &crcHashModeGet, &crcSeedGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersGet: %d", prvTgfDevNum);

    /* set Enhanced CRC-Based Hash Mode */
    rc =  prvTgfTrunkHashCrcParametersSet(PRV_TGF_TRUNK_LBH_CRC_32_MODE_E, crcSeed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);

    for (i = 0; i < 70; i++)
    {
        rc =  prvTgfTrunkHashCrcSaltByteSet(prvTgfDevNum,i,saltValue);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcSaltByteSet: %d", prvTgfDevNum);
    }
    maskEntry.macDaMaskBmp = 0x3f;
    maskEntry.macSaMaskBmp = 0x3f;
    maskEntry.ipDipMaskBmp = 0xffff;
    maskEntry.ipSipMaskBmp = 0xffff;
    /* set HASH mask */
    rc =  prvTgfTrunkHashMaskCrcEntrySet(GT_FALSE,0,packetType,&maskEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcSaltByteSet: %d", prvTgfDevNum);


    /* make ecmp settings */
    ecmpEntry.numOfPaths = prvTgfNumOfPaths;
    ecmpEntry.randomEnable = randomEnable;
    ecmpEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    ecmpEntry.multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    rc = prvTgfIpEcmpEntryWrite(prvTgfDevNum, prvTgfEcmpEntryBaseIndex, &ecmpEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpEntryWrite: %d", prvTgfDevNum);

    /*  Get the start bit and the number of bits needed by the L3 ECMP hash mechanism*/
    rc =  prvTgfIpEcmpHashNumBitsGet(prvTgfDevNum, &startBitGet,&numOfBitsGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpHashNumBitsGet: %d", prvTgfDevNum);
    rc =  prvTgfIpEcmpHashNumBitsSet(prvTgfDevNum,16,16);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpHashNumBitsSet: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* Create prvTgfNumOfPaths route enties (100)
     * The rules are following:
     *   pathIndex          = [0...prvTgfNumOfPaths-1]
     *   baseRouteEntryIndex = prvTgfRouteEntryBaseIndex + pathIndex
     *   nextHopARPPointer   = prvTgfArpEntryBaseIndex   + pathIndex
     *
     */
    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
    {

        nextHopPort= (pathIndex%3)+1;

        /* write a ARP MAC address to the Router ARP Table */
        prvTgfArpMac[5] = (GT_U8) (prvTgfArpEntryBaseIndex + pathIndex);

        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));


        /* AUTODOC: write 8 ARP MACs 00:00:00:11:22:[01..100]  to the Router ARP Table */
        rc = prvTgfIpRouterArpAddrWrite(prvTgfArpEntryBaseIndex + pathIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* setup default values for the route entries */
        cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
        regularEntryPtr = &routeEntriesArray[0];
        regularEntryPtr->cmd               = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
        regularEntryPtr->nextHopVlanId     = PRV_TGF_NEXTHOPE_VLANID_CNS;
        regularEntryPtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
        regularEntryPtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
        regularEntryPtr->nextHopInterface.devPort.portNum =
                            prvTgfPortsArray[nextHopPort];

        /* setup the route entries by incremental values */
        regularEntryPtr->countSet          = (CPSS_IP_CNT_SET_ENT) pathIndex % 4;
        regularEntryPtr->nextHopARPPointer = prvTgfArpEntryBaseIndex + pathIndex;

        /* AUTODOC: add 8 UC route entries with nexthop VLAN 6 and nexthop port 1,2,0r 3 */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex + pathIndex,
                                         routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

        /* AUTODOC: read and check the UC Route entry from the Route Table */
        cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex + pathIndex,
                                        routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthop info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_MULTIPATH_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }

    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfEcmpEntryBaseIndex;

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3 /32 and numOfPaths 100 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

}


/**
* @internal prvTgfBasicIpv4UcEcmpRoutingRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingRouteConfigurationSet
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
            prvTgfBasicIpv4UcEcmpRoutingLttRouteConfigurationSet(randomEnable,indirectAcessEnable);
            break;
        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:
            break;
    }
}

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingHashCheckRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route and hashing Configuration
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingHashCheckRouteConfigurationSet
(
    GT_BOOL randomEnable
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
            prvTgfBasicIpv4UcEcmpRoutingHashCheckLttRouteConfigurationSet(randomEnable);
            break;
        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:
            break;
    }
}

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingTrafficGenerate
(
    GT_BOOL randomEnable
)
{
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
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

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

    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
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
                prvTgfArpMac[5] = (GT_U8) ((prvTgfArpEntryBaseIndex + prvTgfSipBaseIndex + pathIndex)
                                            & (prvTgfNumOfPaths - 1));
                prvTgfPacketIpv4Part.srcAddr[3] = (GT_U8) (prvTgfSipBaseIndex + pathIndex * prvTgfSipStep);

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
            prvTgfArpMac[5] = (GT_U8) (prvTgfArpEntryBaseIndex + pathIndex);
            prvTgfSipBaseIndex        = 4;
            prvTgfSipStep             = 1;
            prvTgfPacketIpv4Part.srcAddr[3] = (GT_U8) (prvTgfSipBaseIndex    + pathIndex * prvTgfSipStep);

            /* AUTODOC: send 8 IPv4 packets from port 0 with: */
            /* AUTODOC: DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
            /* AUTODOC: VID=5, srcIPs=2.2.2.[4.6.8.0xa.0xc.0xE.0x10,0x12], dstIP=1.1.1.3 */
        }

        /* send packet */
        PRV_UTF_LOG4_MAC("Send packet: SIP=%x, DA=%x, RouteEntry=%x, ArpEntry=%x\n",
                         prvTgfPacketIpv4Part.srcAddr[3],
                         prvTgfArpMac[5],
                         prvTgfRouteEntryBaseIndex + pathIndex,
                         prvTgfArpEntryBaseIndex + pathIndex);


        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify routed packet on port 3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            expPktsRcv = expPktsSent = (GT_U8) (portIter == PRV_TGF_SEND_PORT_IDX_CNS ||
                                   portIter == PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS);

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
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

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
                    prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                    prvTgfArpMac[3], prvTgfArpMac[4]);
        }
        else
        {
            UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
                    "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                    prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);
        }
    }

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters %d:\n", portIter);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingHashCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] randomEnable             - whether the ECMP entry is set with  == GT_TRUE
*                                       None
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingHashCheckTrafficGenerate
(
    GT_BOOL randomEnable
)
{
    GT_STATUS                               rc          = GT_OK;
    GT_U32                                  portIter    = 0;
    GT_U32                                  numTriggers = 0;
    TGF_VFD_INFO_STC                        vfdArray[1];
    CPSS_INTERFACE_INFO_STC                 portInterface;
    PRV_TGF_IP_COUNTER_SET_STC              ipCounters;
    GT_U8                                   pathIndex;
    GT_U8                                   expPktsRcv;
    GT_U8                                   expPktsSent;
    GT_U32                                  nextHopPort;
    GT_U32                                  hashIndex;
    PRV_TGF_TRUNK_LBH_INPUT_DATA_STC        hashInputData;
    GT_U32                                  i;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic for ECMP =======\n");

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 10);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);
    /* configure MAC DA and MAC SA input data */
    cpssOsMemSet(&hashInputData,0,sizeof(hashInputData));
    for ( i = 0; i < 6 ; i++)
    {
        hashInputData.macDaArray[i] = prvTgfPacketL2Part.daMac[i];
        hashInputData.macSaArray[i] = prvTgfPacketL2Part.saMac[i];
    }

    /* configure ip4 dip and ip4 sip */
    for ( i = 0; i < 4; i++)
    {
        hashInputData.ipDipArray[12+i] = prvTgfPacketIpv4Part.dstAddr[i];
        hashInputData.ipSipArray[12+i] = prvTgfPacketIpv4Part.srcAddr[i];
    }

    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
    {
        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");


        /* reset IP couters and set ROUTE_ENTRY mode */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* enable capture on nexthope port */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        hashInputData.ipSipArray[12+3] = (GT_U8) (prvTgfSipBaseIndex + pathIndex * prvTgfSipStep);
        /* calculate input ecmp hash value */
        rc =  prvTgfTrunkHashIndexCalculate(prvTgfDevNum,PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,&hashInputData,
                                            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E,prvTgfNumOfPaths,&hashIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTrunkHashIndexCalculate: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("hashIndex = [%d] \n",hashIndex);

        nextHopPort= (hashIndex%3)+1;
        portInterface.devPort.portNum = prvTgfPortsArray[nextHopPort];

        PRV_UTF_LOG2_MAC("expected egress portNum = [%d] (portIdx=[%d]) \n",
            portInterface.devPort.portNum,
            nextHopPort);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture before sending the traffic */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* modify the packet */
        prvTgfArpMac[5] = (GT_U8) (hashIndex);
        prvTgfPacketIpv4Part.srcAddr[3] = (GT_U8) (prvTgfSipBaseIndex + pathIndex * prvTgfSipStep);

        /* AUTODOC: send  IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
        /* AUTODOC:   VID=5, srcIPs=2.2.2.[1..100], dstIP=1.1.1.3 */

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* send packet */
        PRV_UTF_LOG4_MAC("\n\nSend packet: SIP=%x, DA=%x, RouteEntry=%x, ArpEntry=%x\n\n",
                         prvTgfPacketIpv4Part.srcAddr[3],
                         prvTgfArpMac[5],
                         prvTgfRouteEntryBaseIndex + hashIndex,
                         prvTgfArpEntryBaseIndex + hashIndex);


        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify routed packet  */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            expPktsRcv = expPktsSent = (GT_U8) (portIter == PRV_TGF_SEND_PORT_IDX_CNS ||
                                   portIter == nextHopPort);

            /* prevent "flip flopping" behavior */
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
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

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
                    prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                    prvTgfArpMac[3], prvTgfArpMac[4]);
        }
        else
        {
            UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
                    "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                    prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);
        }

        if(GT_TRUE == prvUtfIsGmCompilation())
        {
            /* reduce time for GM (factor 10) */
            pathIndex += 9;
        }

    }

    /* restore rvTgfArpMac[5] to 0*/
    prvTgfArpMac[5] = 0;

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters %d:\n", portIter);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] enhancedHashCrcMode      - hash mode
*                                      GT_FALSE - default legacy mode
*                                      GT_TRUE  - enhanced CRC hash mode
*                                       None
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4UcEcmpRoutingConfigurationRestore
(
    GT_BOOL enhancedHashCrcMode
)
{
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    GT_U32      portIter  = 0;
    PRV_TGF_IP_ECMP_ENTRY_STC ecmpEntry;
    GT_U8       pathIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* Clear the ECMP entry */
    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) == GT_TRUE) || 
        ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_TRUE)))
    {
        ecmpEntry.numOfPaths = 1;
        ecmpEntry.randomEnable = GT_FALSE;
        ecmpEntry.routeEntryBaseIndex = 0;
        ecmpEntry.multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
        rc = prvTgfIpEcmpEntryWrite(prvTgfDevNum, prvTgfEcmpEntryBaseIndex, &ecmpEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpEntryWrite: %d", prvTgfDevNum);
    }

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    if (enhancedHashCrcMode == GT_TRUE)
    {
        /* AUTODOC: restore enhanced crc hash data */
        /* restore global hash mode */
        rc =  prvTgfTrunkHashGlobalModeSet(globalHashModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

        /* restore crc hash parameters */
        rc =  prvTgfTrunkHashCrcParametersSet(crcHashModeGet, crcSeedGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);

        /* restore ecmp  start bit and the number of bits needed by the L3 ECMP hash mechanism */
        rc =  prvTgfIpEcmpHashNumBitsSet(prvTgfDevNum, startBitGet,numOfBitsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpHashNumBitsSet: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: disable Trunk Hash Ip Mode */
        rc = prvTgfTrunkHashIpModeSet(GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashIpModeSet: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* delete Ports 0,1 from VLAN 5 */
    for (portIter = 0; portIter < (PRV_TGF_PORT_COUNT_CNS/2); portIter++)
    {
        /* delete port from VLAN */
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                       prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                     prvTgfPortsArray[portIter]);
    }

    /* delete Ports 0,1,2,3 from VLAN 6 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* delete port from VLAN */
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_NEXTHOPE_VLANID_CNS,
                                       prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_NEXTHOPE_VLANID_CNS,
                                     prvTgfPortsArray[portIter]);
    }

    if (indirectAcessEnableRestore == GT_TRUE)
    {
        /* restore ECPM indirect not hop entries */
        for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
        {
            rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum,prvTgfRouteEntryBaseIndex + pathIndex, nextHopIndexRestore[pathIndex]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntrySet: %d", prvTgfDevNum);
        }
    }

    indirectAcessEnableRestore = GT_FALSE;

   /* invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 5");

    /* invalidate NEXTHOPE VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 6");
}


