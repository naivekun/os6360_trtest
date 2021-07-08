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
* @file prvTgfIpv6UcRoutingAddMany.c
*
* @brief IPV6 UC Routing with prefix manipulations to fit compress_1,
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
#include <ip/prvTgfIpv6UcRoutingAddMany.h>
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

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

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
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0x1111, 0x1111}  /* TGF_IPV6_ADDR dstAddr */
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
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerate()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPV6ADDR                             ipAddr;
    GT_U32                                  i=0;



    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv4 UC prefix 1122:0000:0000:0000:0000:0000:ccdd:eeff/128 */

    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0101/128 octet 3 is compress_1 bucket */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0102/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0103/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0104/128 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0105/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0106/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0107/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0108/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0109/128 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0110/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0111/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0201/128 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0301/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0401/128 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0501/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0601/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0701/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0801/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0901/128 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:1001/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:1101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0102:0101/128 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0103:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0104:0101/128 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0105:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0106:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0107:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0108:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0109:0101/128 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:01010:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:01011:0101/128 */


    /* -------------------------------------------------------------------------
     * 3. Create a new IPv6 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipAddr.arIP[0]=0x11;
    ipAddr.arIP[1]=0x22;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;
    ipAddr.arIP[4]=0;
    ipAddr.arIP[5]=0;
    ipAddr.arIP[6]=0;
    ipAddr.arIP[7]=0;
    ipAddr.arIP[8]=0;
    ipAddr.arIP[9]=0;
    ipAddr.arIP[10]=0;
    ipAddr.arIP[11]=0;
    ipAddr.arIP[12]=0xcc;
    ipAddr.arIP[13]=0xdd;
    ipAddr.arIP[14]=0xee;
    ipAddr.arIP[15]=0xff;


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);

    ipAddr.arIP[0]=0x11;
    ipAddr.arIP[1]=0x22;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;
    ipAddr.arIP[4]=0;
    ipAddr.arIP[5]=0;
    ipAddr.arIP[6]=0;
    ipAddr.arIP[7]=0;
    ipAddr.arIP[8]=0;
    ipAddr.arIP[9]=0;
    ipAddr.arIP[10]=0;
    ipAddr.arIP[11]=0;
    ipAddr.arIP[12]=1;
    ipAddr.arIP[13]=1;
    ipAddr.arIP[14]=1;
    ipAddr.arIP[15]=1;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 128, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);

    /* AUTODOC: add 29 IPv4 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[12]=1;
            ipAddr.arIP[13]=1;
            ipAddr.arIP[14]=1;
            ipAddr.arIP[15]=1;
        }

        ipAddr.arIP[(((i-1)/10)+13)]++;


        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 128, &nextHopInfo, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);

    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[12]=1;
            ipAddr.arIP[13]=1;
            ipAddr.arIP[14]=1;
            ipAddr.arIP[15]=1;
        }

        ipAddr.arIP[(((i-1)/10)+13)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);
    }
}

/**
* @internal prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv6UcRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPV6ADDR                             ipAddr;
    GT_U32                                  i=0;
    GT_BOOL                                 activityBit;
    GT_BOOL                                 activityState;



    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:ccdd:eeff/128 */

    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0101/128 octet 3 is compress_1 bucket */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0102/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0103/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0104/128 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0105/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0106/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0107/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0108/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0109/128 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0110/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0111/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0201/128 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0301/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0401/128 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0501/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0601/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0701/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0801/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:0901/128 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:1001/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0101:1101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0102:0101/128 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0103:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0104:0101/128 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0105:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0106:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0107:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0108:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:0109:0101/128 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:01010:0101/128 */
    /* AUTODOC: add IPv6 UC prefix 1122:0000:0000:0000:0000:0000:01011:0101/128 */

    prvTgfIpLpmActivityBitEnableGet(&activityBit);
    prvTgfIpLpmActivityBitEnableSet(GT_TRUE);

    /* -------------------------------------------------------------------------
     * 3. Create a new IPv6 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipAddr.arIP[0]=0x11;
    ipAddr.arIP[1]=0x22;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;
    ipAddr.arIP[4]=0;
    ipAddr.arIP[5]=0;
    ipAddr.arIP[6]=0;
    ipAddr.arIP[7]=0;
    ipAddr.arIP[8]=0;
    ipAddr.arIP[9]=0;
    ipAddr.arIP[10]=0;
    ipAddr.arIP[11]=0;
    ipAddr.arIP[12]=0xcc;
    ipAddr.arIP[13]=0xdd;
    ipAddr.arIP[14]=0xee;
    ipAddr.arIP[15]=0xff;


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);

    rc = prvTgfIpLpmIpv6UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 128, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d ipAddr.u32Ip[3] = %#x", activityState, ipAddr.u32Ip[3]);

    ipAddr.arIP[0]=0x11;
    ipAddr.arIP[1]=0x22;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;
    ipAddr.arIP[4]=0;
    ipAddr.arIP[5]=0;
    ipAddr.arIP[6]=0;
    ipAddr.arIP[7]=0;
    ipAddr.arIP[8]=0;
    ipAddr.arIP[9]=0;
    ipAddr.arIP[10]=0;
    ipAddr.arIP[11]=0;
    ipAddr.arIP[12]=1;
    ipAddr.arIP[13]=1;
    ipAddr.arIP[14]=1;
    ipAddr.arIP[15]=1;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 128, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);

    rc = prvTgfIpLpmIpv6UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 128, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d ipAddr.u32Ip[3] = %#x", activityState, ipAddr.u32Ip[3]);

    /* AUTODOC: add 29 IPv4 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[12]=1;
            ipAddr.arIP[13]=1;
            ipAddr.arIP[14]=1;
            ipAddr.arIP[15]=1;
        }

        ipAddr.arIP[(((i-1)/10)+13)]++;


        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 128, &nextHopInfo, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);

        rc = prvTgfIpLpmIpv6UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 128, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d ipAddr.u32Ip[3] = %#x", activityState, ipAddr.u32Ip[3]);
    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[12]=1;
            ipAddr.arIP[13]=1;
            ipAddr.arIP[14]=1;
            ipAddr.arIP[15]=1;
        }

        ipAddr.arIP[(((i-1)/10)+13)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6UcRoutingAddManyTrafficGenerate(ipAddr);

        rc = prvTgfIpLpmIpv6UcPrefixActivityStatusGet(prvTgfLpmDBId, 0, ipAddr, 128, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, activityState, "prvTgfIpLpmIpv6UcPrefixActivityStatusGet: %d pAddr.u32Ip[3] = %#x 2", activityState, ipAddr.u32Ip[3]);
    }

    prvTgfIpLpmActivityBitEnableSet(activityBit);
}

/**
* @internal prvTgfIpv6UcRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] ipAddr                   - ip address
*                                       None
*/
GT_VOID prvTgfIpv6UcRoutingAddManyTrafficGenerate
(
    GT_IPV6ADDR         ipAddr
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
    GT_U32                          i=0;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

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
            /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode for all ports */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* fill destination IP address for packet */

    for (i = 0; i < 8; i++)
    {
        prvTgfPacketIpv6Part.dstAddr[i] = (ipAddr.arIP[i * 2 +1]);
        prvTgfPacketIpv6Part.dstAddr[i] |= ((GT_U16)(ipAddr.arIP[i * 2]))<<8;
    }

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);

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

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send IPv6 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=6545:0000:0000:0000:0000:0000:0000:3212 */
    /* AUTODOC:   dstIP=1122:0000:0000:0000:0000:0000:ccdd:eeff */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    cpssOsTimerWkAfter(200);

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

        /* AUTODOC: verify routed packet on port 3 with: */
        /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:06 */
        switch (portIter) {
            case PRV_TGF_SEND_PORT_IDX_CNS:
            case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

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


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac */
    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
            prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);


    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);
}

/**
* @internal prvTgfIpv6UcRoutingAddManyConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note Restore Route Configuration
*
*/
GT_VOID prvTgfIpv6UcRoutingAddManyConfigurationRestore()
{
    GT_STATUS rc=GT_OK;
    GT_U32 i=0;
    GT_IPV6ADDR ipAddr;

    /* -------------------------------------------------------------------------
     * 1. Restore Route Configuration
     */

    ipAddr.arIP[0]=0x11;
    ipAddr.arIP[1]=0x22;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=0;
    ipAddr.arIP[4]=0;
    ipAddr.arIP[5]=0;
    ipAddr.arIP[6]=0;
    ipAddr.arIP[7]=0;
    ipAddr.arIP[8]=0;
    ipAddr.arIP[9]=0;
    ipAddr.arIP[10]=0;
    ipAddr.arIP[11]=0;
    ipAddr.arIP[12]=1;
    ipAddr.arIP[13]=1;
    ipAddr.arIP[14]=1;
    ipAddr.arIP[15]=1;

    rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 128);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel: %d", prvTgfDevNum);


    /* AUTODOC: add 30 IPv6 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipAddr.arIP[12]=1;
            ipAddr.arIP[13]=1;
            ipAddr.arIP[14]=1;
            ipAddr.arIP[15]=1;
        }

        ipAddr.arIP[(((i-1)/10)+13)]++;

        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 128);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel: %d", prvTgfDevNum);
    }
}



