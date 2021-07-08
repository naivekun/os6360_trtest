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
* @file prvTgfIpv4UcRoutingAddMany.c
*
* @brief IPV4 UC Routing with prefix manipulations to fit compress_1,
* compress_2 and regular buckets
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpv4UcRoutingAddMany.h>
#include <trunk/prvTgfTrunk.h>

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

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U16        prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   1000

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

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
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
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

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerate()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  i=0;
    GT_BOOL                                 defragmentationEnable=GT_TRUE;



    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv4 UC prefix 1.1.1.1/32 octet 3 is compress_1 bucket */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.2/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 already added */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.4/32 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.5/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.6/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.7/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.8/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.9/32 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.10/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.11/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.2.1/32 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv4 UC prefix 1.1.3.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.4.1/32 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 UC prefix 1.1.5.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.6.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.7.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.8.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.9.1/32 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv4 UC prefix 1.1.10.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.11.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.2.1.1/32 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv4 UC prefix 1.3.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.4.1.1/32 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 UC prefix 1.5.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.6.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.7.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.8.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.9.1.1/32 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv4 UC prefix 1.10.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.11.1.1/32 */


    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipAddr.arIP[0]=1;
    ipAddr.arIP[1]=1;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=3;

    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    ipAddr.arIP[0]=1;
    ipAddr.arIP[1]=1;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=1;

    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, defragmentationEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);


    /* AUTODOC: add 29 IPv4 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[0]=1;
            ipAddr.arIP[1]=1;
            ipAddr.arIP[2]=1;
            ipAddr.arIP[3]=1;
        }

        ipAddr.arIP[(((i-1)/10)+1)]++;

        /* prefix 1.1.1.3 was already added */
        if(!((ipAddr.arIP[0]==1) && (ipAddr.arIP[1]==1) && (ipAddr.arIP[2]==1) && (ipAddr.arIP[3]==3)))
        {
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, defragmentationEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

            /* AUTODOC: send packet to the prefix added */
            prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

        }
    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[0]=1;
            ipAddr.arIP[1]=1;
            ipAddr.arIP[2]=1;
            ipAddr.arIP[3]=1;
        }

        ipAddr.arIP[(((i-1)/10)+1)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);
    }


}

/**
* @internal prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration, generate traffic
*         and check LPM Activity State
*/
GT_VOID prvTgfIpv4UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  i=0;
    GT_BOOL                                 activityBit;
    GT_BOOL                                 activityState;
    GT_BOOL                                 defragmentationEnable=GT_TRUE;



    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv4 UC prefix 1.1.1.1/32 octet 3 is compress_1 bucket */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.2/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 already added */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.4/32 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.5/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.6/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.7/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.8/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.9/32 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.10/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.1.11/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.2.1/32 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv4 UC prefix 1.1.3.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.4.1/32 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 UC prefix 1.1.5.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.6.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.7.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.8.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.9.1/32 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv4 UC prefix 1.1.10.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.1.11.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.2.1.1/32 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv4 UC prefix 1.3.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.4.1.1/32 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 UC prefix 1.5.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.6.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.7.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.8.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.9.1.1/32 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv4 UC prefix 1.10.1.1/32 */
    /* AUTODOC: add IPv4 UC prefix 1.11.1.1/32 */

    prvTgfIpLpmActivityBitEnableGet(&activityBit);
    prvTgfIpLpmActivityBitEnableSet(GT_TRUE);

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipAddr.arIP[0]=1;
    ipAddr.arIP[1]=1;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=3;

    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    rc = prvTgfIpLpmIpv4UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 32, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d", activityState);

    ipAddr.arIP[0]=1;
    ipAddr.arIP[1]=1;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=1;

    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, defragmentationEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

    rc = prvTgfIpLpmIpv4UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 32, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d", activityState);

    /* AUTODOC: add 29 IPv4 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[0]=1;
            ipAddr.arIP[1]=1;
            ipAddr.arIP[2]=1;
            ipAddr.arIP[3]=1;
        }

        ipAddr.arIP[(((i-1)/10)+1)]++;

        /* prefix 1.1.1.3 was already added */
        if(!((ipAddr.arIP[0]==1) && (ipAddr.arIP[1]==1) && (ipAddr.arIP[2]==1) && (ipAddr.arIP[3]==3)))
        {
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, defragmentationEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

            /* AUTODOC: send packet to the prefix added */
            prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

            rc = prvTgfIpLpmIpv4UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 32, GT_TRUE, &activityState);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d", rc);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d ipAddr.u32Ip = %#x", activityState, ipAddr.u32Ip);
        }
    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[0]=1;
            ipAddr.arIP[1]=1;
            ipAddr.arIP[2]=1;
            ipAddr.arIP[3]=1;
        }

        ipAddr.arIP[(((i-1)/10)+1)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4UcRoutingAddManyTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], ipAddr, PRV_TGF_NEXTHOPE_VLANID_CNS);

        rc = prvTgfIpLpmIpv4UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 32, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv4UcPrefixActivityStatusGet: %d pAddr.u32Ip = %#x 2", activityState, ipAddr.u32Ip);
    }

    prvTgfIpLpmActivityBitEnableSet(activityBit);
}

/**
* @internal prvTgfIpv4UcRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] ipAddr                   - ip adders of the packet
* @param[in] nextHopVlanId            - nextHop VLAN
*                                       None
*/
GT_VOID prvTgfIpv4UcRoutingAddManyTrafficGenerate
(
    GT_U32               sendPortNum,
    GT_U32               nextHopPortNum,
    GT_IPADDR           ipAddr,
    GT_U16              nextHopVlanId
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[2];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
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

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));


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
    trunkMember.port = nextHopPortNum;
    trunkMember.hwDevice = prvTgfDevNum;

    rc = prvTgfTrunkDbIsMemberOfTrunk(prvTgfDevNum,&trunkMember,&nextHopTrunkId);
    if(rc == GT_OK)
    {
        /* the port is member of the trunk */

        /* setup nexthope portInterface for capturing */
        portInterface.type            = CPSS_INTERFACE_TRUNK_E;
        portInterface.trunkId         = nextHopTrunkId;

        /*since we have 2 members in the NH trunk send 8 packets*/
        prvTgfBurstCount = 8;

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
        /*since we have 4 members in the NH trunk send 16 packets*/
        prvTgfBurstCount = 16;

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
           traffic after the rout will be captured to the CPU.
           */
        captureType = TGF_CAPTURE_MODE_PCL_E;

        /* state that no need to capture packets that are in the Vlan before the routing */
        rc = tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet(GT_TRUE,PRV_TGF_SEND_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCapturePclExcludeVidSet: %d, %d\n",
                                     prvTgfDevNum, nextHopPortNum);
    }
    else
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;
    }


    /* enable capture on next hop port/trunk */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* fill destination IP address for packet */
    cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, ipAddr.arIP, sizeof(prvTgfPacketIpv4Part.dstAddr));

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
    /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=1.1.1.3 */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);

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
        if( (prvTgfPortsArray[portIter]==sendPortNum) || (prvTgfPortsArray[portIter]==nextHopPortNum))
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
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* get trigger counters where packet has VLAN as nextHopVlanId */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = TGF_L2_HEADER_SIZE_CNS;
    vfdArray[1].cycleCount = 4;
    vfdArray[1].patternPtr[0] = 0x81;
    vfdArray[1].patternPtr[1] = 0x00;
    vfdArray[1].patternPtr[2] = (GT_U8)(nextHopVlanId >> 8);
    vfdArray[1].patternPtr[3] = (GT_U8)(nextHopVlanId & 0xFF);

    /* disable capture on nexthope port , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, nextHopPortNum);

    /* check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
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

    /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
    /* number of triggers should be according to number of transmit*/
    UTF_VERIFY_EQUAL6_STRING_MAC(((1<<prvTgfBurstCount)-1), numTriggers & BIT_0,
             "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

    /* AUTODOC: check if captured packet has expected VLAN as nextHopVlanId */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_1) >> 1,
                                 "\n   VLAN of captured packet must be: %02X",
                                 nextHopVlanId);
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
                                 prvTgfDevNum, nextHopPortNum);

    /* just for 'cleanup' */
    captureType = TGF_CAPTURE_MODE_MIRRORING_E;
}

/**
* @internal prvTgfIpv4UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpv4UcRoutingAddManyConfigurationRestore()
{
    GT_STATUS rc=GT_OK;
    GT_U32 i=0;
    GT_IPADDR ipAddr;

    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    ipAddr.arIP[0]=1;
    ipAddr.arIP[1]=1;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=1;

    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);


    /* AUTODOC: add 30 IPv4 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[0]=1;
            ipAddr.arIP[1]=1;
            ipAddr.arIP[2]=1;
            ipAddr.arIP[3]=1;
        }

        ipAddr.arIP[(((i-1)/10)+1)]++;

        /* 1.1.1.3 was already deleted in prvTgfBasicIpv4UcRoutingConfigurationRestore */
        if(!((ipAddr.arIP[0]==1) && (ipAddr.arIP[1]==1) && (ipAddr.arIP[2]==1) && (ipAddr.arIP[3]==3)))
        {
            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
        }
    }
}



