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
* @file prvTgfIpv6McRoutingAddMany.c
*
* @brief IPV6 MC Routing with prefix manipulations to fit compress_1,
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
#include <ip/prvTgfIpv6McRoutingAddMany.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* send VLAN */
#define PRV_TGF_SEND_VLANID_CNS           80

/* multicast subscriber 1 VLAN */
#define PRV_TGF_MC_1_VLANID_CNS           55

/* multicast subscriber 2 VLAN */
#define PRV_TGF_MC_2_VLANID_CNS           17

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         2

/* port index for subscriber 1 */
#define PRV_TGF_MC_1_PORT_IDX_CNS         1

/* port index for subscriber 2 */
#define PRV_TGF_MC_2_PORT_IDX_CNS         3

/* port index for subscriber 3 */
#define PRV_TGF_MC_3_PORT_IDX_CNS         0

/* default vidx number */
#define PRV_TGF_DEF_VIDX_CNS              0

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* the Route entry index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 10;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* expected number of sent packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountRxMC1 = 0;
static GT_U8 prvTgfPacketsCountRxMC2 = 1;
static GT_U8 prvTgfPacketsCountRxMC3 = 0;

/* expected number of sent packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountTxMC1 = 1;
static GT_U8 prvTgfPacketsCountTxMC2 = 1;
static GT_U8 prvTgfPacketsCountTxMC3 = 1;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x33, 0x33, 0x00, 0x00, 0x22, 0x22},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
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

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerate()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_LTT_ENTRY_STC                mcRouteLttEntry;
    GT_IPV6ADDR                             ipGroup;
    GT_IPV6ADDR                             ipSrc;
    GT_U32                                  i=0;



    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0101/128 octet 3 is compress_1 bucket */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0102/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0103/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0104/128 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0105/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0106/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0107/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0108/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0109/128 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0110/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0111/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0201/128 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0301/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0401/128 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0501/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0601/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0701/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0801/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0901/128 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:1001/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:1101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0102:0101/128 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0103:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0104:0101/128 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0105:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0106:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0107:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0108:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0109:0101/128 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:01010:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:01011:0101/128 */


    /* -------------------------------------------------------------------------
     * 3. Create a new IPv6 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipSrc.arIP[0]=0x22;
    ipSrc.arIP[1]=0x22;
    ipSrc.arIP[2]=0;
    ipSrc.arIP[3]=0;
    ipSrc.arIP[4]=0;
    ipSrc.arIP[5]=0;
    ipSrc.arIP[6]=0;
    ipSrc.arIP[7]=0;
    ipSrc.arIP[8]=0;
    ipSrc.arIP[9]=0;
    ipSrc.arIP[10]=0;
    ipSrc.arIP[11]=0;
    ipSrc.arIP[12]=0;
    ipSrc.arIP[13]=0;
    ipSrc.arIP[14]=0x22;
    ipSrc.arIP[15]=0x22;

    ipGroup.arIP[0]=0xFF;
    ipGroup.arIP[1]=0;
    ipGroup.arIP[2]=0;
    ipGroup.arIP[3]=0;
    ipGroup.arIP[4]=0;
    ipGroup.arIP[5]=0;
    ipGroup.arIP[6]=0;
    ipGroup.arIP[7]=0;
    ipGroup.arIP[8]=0;
    ipGroup.arIP[9]=0;
    ipGroup.arIP[10]=0;
    ipGroup.arIP[11]=0;
    ipGroup.arIP[12]=0;
    ipGroup.arIP[13]=0;
    ipGroup.arIP[14]=0x22;
    ipGroup.arIP[15]=0x22;


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);

    ipSrc.arIP[0]=0x11;
    ipSrc.arIP[1]=0x22;
    ipSrc.arIP[2]=0;
    ipSrc.arIP[3]=0;
    ipSrc.arIP[4]=0;
    ipSrc.arIP[5]=0;
    ipSrc.arIP[6]=0;
    ipSrc.arIP[7]=0;
    ipSrc.arIP[8]=0;
    ipSrc.arIP[9]=0;
    ipSrc.arIP[10]=0;
    ipSrc.arIP[11]=0;
    ipSrc.arIP[12]=1;
    ipSrc.arIP[13]=1;
    ipSrc.arIP[14]=1;
    ipSrc.arIP[15]=1;


    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: %d", prvTgfDevNum);


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);

    /* AUTODOC: add 29 IPv4 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[12]=1;
            ipSrc.arIP[13]=1;
            ipSrc.arIP[14]=1;
            ipSrc.arIP[15]=1;
        }

        ipSrc.arIP[(((i-1)/10)+13)]++;

        rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);


    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[12]=1;
            ipSrc.arIP[13]=1;
            ipSrc.arIP[14]=1;
            ipSrc.arIP[15]=1;
        }

        ipSrc.arIP[(((i-1)/10)+13)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);
    }
}

/**
* @internal prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv6McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_LTT_ENTRY_STC                mcRouteLttEntry;
    GT_IPV6ADDR                             ipGroup;
    GT_IPV6ADDR                             ipSrc;
    GT_U32                                  i=0;
    GT_BOOL                                 activityBit;
    GT_BOOL                                 activityState;



    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0101/128 octet 3 is compress_1 bucket */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0102/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0103/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0104/128 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0105/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0106/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0107/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0108/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0109/128 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0110/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0111/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0201/128 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0301/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0401/128 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0501/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0601/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0701/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0801/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:0901/128 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:1001/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0101:1101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0102:0101/128 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0103:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0104:0101/128 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0105:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0106:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0107:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0108:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:0109:0101/128 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:01010:0101/128 */
    /* AUTODOC: add IPv6 MC prefix 1122:0000:0000:0000:0000:0000:01011:0101/128 */

    prvTgfIpLpmActivityBitEnableGet(&activityBit);
    prvTgfIpLpmActivityBitEnableSet(GT_TRUE);

    /* -------------------------------------------------------------------------
     * 3. Create a new IPv6 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipSrc.arIP[0]=0x22;
    ipSrc.arIP[1]=0x22;
    ipSrc.arIP[2]=0;
    ipSrc.arIP[3]=0;
    ipSrc.arIP[4]=0;
    ipSrc.arIP[5]=0;
    ipSrc.arIP[6]=0;
    ipSrc.arIP[7]=0;
    ipSrc.arIP[8]=0;
    ipSrc.arIP[9]=0;
    ipSrc.arIP[10]=0;
    ipSrc.arIP[11]=0;
    ipSrc.arIP[12]=0;
    ipSrc.arIP[13]=0;
    ipSrc.arIP[14]=0x22;
    ipSrc.arIP[15]=0x22;

    ipGroup.arIP[0]=0xFF;
    ipGroup.arIP[1]=0;
    ipGroup.arIP[2]=0;
    ipGroup.arIP[3]=0;
    ipGroup.arIP[4]=0;
    ipGroup.arIP[5]=0;
    ipGroup.arIP[6]=0;
    ipGroup.arIP[7]=0;
    ipGroup.arIP[8]=0;
    ipGroup.arIP[9]=0;
    ipGroup.arIP[10]=0;
    ipGroup.arIP[11]=0;
    ipGroup.arIP[12]=0;
    ipGroup.arIP[13]=0;
    ipGroup.arIP[14]=0x22;
    ipGroup.arIP[15]=0x22;


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);

    /* AUTODOC: check Activity State */
    rc = prvTgfIpLpmIpv6McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
            "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d ipSrc.u32Ip[3] = %#x ipGroup.u32Ip[3] = %#x", activityState, ipSrc.u32Ip[3], ipGroup.u32Ip[3]);

    ipSrc.arIP[0]=0x11;
    ipSrc.arIP[1]=0x22;
    ipSrc.arIP[2]=0;
    ipSrc.arIP[3]=0;
    ipSrc.arIP[4]=0;
    ipSrc.arIP[5]=0;
    ipSrc.arIP[6]=0;
    ipSrc.arIP[7]=0;
    ipSrc.arIP[8]=0;
    ipSrc.arIP[9]=0;
    ipSrc.arIP[10]=0;
    ipSrc.arIP[11]=0;
    ipSrc.arIP[12]=1;
    ipSrc.arIP[13]=1;
    ipSrc.arIP[14]=1;
    ipSrc.arIP[15]=1;


    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: %d", prvTgfDevNum);


    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);

    rc = prvTgfIpLpmIpv6McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
            "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d ipSrc.u32Ip[3] = %#x ipGroup.u32Ip[3] = %#x", activityState, ipSrc.u32Ip[3], ipGroup.u32Ip[3]);

    /* AUTODOC: add 29 IPv4 UC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[12]=1;
            ipSrc.arIP[13]=1;
            ipSrc.arIP[14]=1;
            ipSrc.arIP[15]=1;
        }

        ipSrc.arIP[(((i-1)/10)+13)]++;

        rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);

        rc = prvTgfIpLpmIpv6McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
                "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d ipSrc.u32Ip[3] = %#x ipGroup.u32Ip[3] = %#x", activityState, ipSrc.u32Ip[3], ipGroup.u32Ip[3]);


    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[12]=1;
            ipSrc.arIP[13]=1;
            ipSrc.arIP[14]=1;
            ipSrc.arIP[15]=1;
        }

        ipSrc.arIP[(((i-1)/10)+13)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv6McRoutingAddManyTrafficGenerate(ipSrc);

        rc = prvTgfIpLpmIpv6McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 128, ipSrc, 128, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
                "prvTgfIpLpmIpv6McEntryActivityStatusGet: %d ipSrc.u32Ip[3] = %#x ipGroup.u32Ip[3] = %#x", activityState, ipSrc.u32Ip[3], ipGroup.u32Ip[3]);
    }

    prvTgfIpLpmActivityBitEnableSet(activityBit);
}


/**
* @internal prvTgfIpv6McRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] srcIpAddr                - source IP Address of the packet sent
*                                       None
*/
GT_VOID prvTgfIpv6McRoutingAddManyTrafficGenerate
(
    GT_IPV6ADDR           srcIpAddr
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInt;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          mllOutMCPkts;
    GT_U32                          oldMllOutMCPktsSet0;
    GT_U32                          oldMllOutMCPktsSet1;
    GT_U32                          mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;
    GT_U32                          i=0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable capture on a MC subscriber port */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
           TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
           "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
           prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS]);

    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    /* set MLL counters mode for MC subscriber 1 */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV6_E;
    cntIntCfg.devNum           = prvTgfDevNum;
    cntIntCfg.portTrunkCntMode = PRV_TGF_IP_PORT_CNT_MODE_E;
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* set MLL counters mode for MC subscriber 2 */
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* fill destination IP address for packet */
    for (i = 0; i < 8; i++)
    {
        prvTgfPacketIpv6Part.srcAddr[i] = (srcIpAddr.arIP[i * 2 +1]);
        prvTgfPacketIpv6Part.srcAddr[i] |= ((GT_U16)(srcIpAddr.arIP[i * 2]))<<8;
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv4 packet from port 2 with: */
    /* AUTODOC:   DA=33:33:cc:dd:ee:ff, SA=00:00:00:00:00:33, VID=5 */
    /* AUTODOC:   dstIp=ff00:0000:0000:0000:0000:0000:0000:2222/128 */
    /* AUTODOC:   srcIp=2222:0000:0000:0000:0000:0000:0000:2222/128 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of StartTransmitting: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    cpssOsTimerWkAfter(100);

    /* disable capture on a MC subscriber port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* AUTODOC: verify routed packet on ports 0,1,3 */
    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS],
            prvTgfPacketsCountRxMC3, prvTgfPacketsCountTxMC3,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS],
            prvTgfPacketsCountRxMC1, prvTgfPacketsCountTxMC1,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS],
            prvTgfPacketsCountRxMC2, prvTgfPacketsCountTxMC2,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    /* get Trigger Counters */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInt.devPort.portNum);

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInt, 1, vfdArray,
                                                        &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
        portInt.devPort.hwDevNum, portInt.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac */
    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
        "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
        prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
        prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

    /* get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n",
                         prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 1: mllOutMCPkts = %d\n", mllOutMCPkts - oldMllOutMCPktsSet0);

    /* get MLL counters for MC subscriber 2 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_1, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 2: mllOutMCPkts = %d\n", mllOutMCPkts - oldMllOutMCPktsSet1);
}



