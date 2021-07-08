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
* @file prvTgfIpv4McRoutingAddMany.c
*
* @brief IPV4 MC Routing with prefix manipulations to fit compress_1,
* compress_2 and regular buckets
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfIpv4McRoutingAddMany.h>


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

#define SECOND_HEMISPHERE_CNS   64

/* is this a call from Dual HW Device test */
static GT_BOOL       isDualHwDevTest = GT_FALSE;

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
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr = ipSrc */
    { 224,1,  1,  1}                   /* dstAddr = ipGrp */
};
/* DATA of packet len = 22 (0x16) */
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

/*************************** Restore config ***********************************/

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerate function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*/
GT_VOID prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerate()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_LTT_ENTRY_STC                mcRouteLttEntry;
    GT_IPADDR                               ipGroup;
    GT_IPADDR                               ipSrc;
    TGF_MAC_ADDR                            daMac;
    TGF_MAC_ADDR                            saMac;
    GT_U32                                  i=0;


    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv4 MC prefix 224.1.1.1/32 octet 3 is compress_1 bucket - already added*/
    /* AUTODOC: add IPv4 MC prefix 224.1.1.2/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.3/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.4/32 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.5/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.6/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.7/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.8/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.9/32 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.10/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.11/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.2.1/32 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv4 MC prefix 224.1.3.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.4.1/32 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 MC prefix 224.1.5.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.6.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.7.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.8.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.9.1/32 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv4 MC prefix 224.1.10.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.11.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.2.1.1/32 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv4 MC prefix 224.3.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.4.1.1/32 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 MC prefix 224.5.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.6.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.7.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.8.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.9.1.1/32 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv4 MC prefix 224.10.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.11.1.1/32 */

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipSrc.arIP[0]=1;
    ipSrc.arIP[1]=1;
    ipSrc.arIP[2]=1;
    ipSrc.arIP[3]=1;

    ipGroup.arIP[0]=224;
    ipGroup.arIP[1]=1;
    ipGroup.arIP[2]=1;
    ipGroup.arIP[3]=1;

    daMac[0]=1;
    daMac[1]=0;
    daMac[2]=0x5E;
    daMac[3]=1;
    daMac[4]=1;
    daMac[5]=1;

    saMac[0]=0;
    saMac[1]=0;
    saMac[2]=0;
    saMac[3]=0;
    saMac[4]=0;
    saMac[5]=0x33;

    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

    /* AUTODOC: add 29 IPv4 MC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipGroup.arIP[0]=224;
            ipGroup.arIP[1]=1;
            ipGroup.arIP[2]=1;
            ipGroup.arIP[3]=1;

            daMac[0]=1;
            daMac[1]=0;
            daMac[2]=0x5E;
            daMac[3]=1;
            daMac[4]=1;
            daMac[5]=1;
        }

        ipGroup.arIP[(((i-1)/10)+1)]++;
        daMac[(((i-1)/10)+3)]++;

        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 32,ipSrc,32, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipGroup.arIP[0]=224;
            ipGroup.arIP[1]=1;
            ipGroup.arIP[2]=1;
            ipGroup.arIP[3]=1;

            daMac[0]=1;
            daMac[1]=0;
            daMac[2]=0x5E;
            daMac[3]=1;
            daMac[4]=1;
            daMac[5]=1;
        }

        ipGroup.arIP[(((i-1)/10)+1)]++;
        daMac[(((i-1)/10)+3)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);
    }


 PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add src IPv4 MC prefix 1.1.1.1/32 octet 3 is compress_1 bucket - already added*/
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.2/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.3/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.4/32 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.5/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.6/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.7/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.8/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.9/32 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.10/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.11/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.2.1/32 octet 2 is compress_1 bucket */
    /* AUTODOC: add src IPv4 MC prefix 1.1.3.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.4.1/32 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.5.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.6.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.7.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.8.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.9.1/32 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add src IPv4 MC prefix 1.1.10.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.11.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.2.1.1/32 octet 1 is compress_1 bucket */
    /* AUTODOC: add src IPv4 MC prefix 1.3.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.4.1.1/32 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add src IPv4 MC prefix 1.5.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.6.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.7.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.8.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.9.1.1/32 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add src IPv4 MC prefix 1.10.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.11.1.1/32 */

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a destination IP address for the prefix */

    ipSrc.arIP[0]=1;
    ipSrc.arIP[1]=1;
    ipSrc.arIP[2]=1;
    ipSrc.arIP[3]=1;

    ipGroup.arIP[0]=224;
    ipGroup.arIP[1]=1;
    ipGroup.arIP[2]=1;
    ipGroup.arIP[3]=1;

    daMac[0]=1;
    daMac[1]=0;
    daMac[2]=0x5E;
    daMac[3]=1;
    daMac[4]=1;
    daMac[5]=1;

    saMac[0]=0;
    saMac[1]=0;
    saMac[2]=0;
    saMac[3]=0;
    saMac[4]=0;
    saMac[5]=0x33;

    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

    /* AUTODOC: add 29 IPv4 MC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[0]=1;
            ipSrc.arIP[1]=1;
            ipSrc.arIP[2]=1;
            ipSrc.arIP[3]=1;

            saMac[0]=0;
            saMac[1]=0;
            saMac[2]=0;
            saMac[3]=0;
            saMac[4]=0;
            saMac[5]=1;
        }

        ipSrc.arIP[(((i-1)/10)+1)]++;
        saMac[(((i-1)/10)+3)]++;

        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 32,ipSrc,32, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[0]=1;
            ipSrc.arIP[1]=1;
            ipSrc.arIP[2]=1;
            ipSrc.arIP[3]=1;

            saMac[0]=0;
            saMac[1]=0;
            saMac[2]=0;
            saMac[3]=0;
            saMac[4]=0;
            saMac[5]=1;
        }

        ipSrc.arIP[(((i-1)/10)+1)]++;
        saMac[(((i-1)/10)+3)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);
    }
}

/**
* @internal prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState function
* @endinternal
*
* @brief   Add Prefixes Configuration and generate traffic
*         and check LPM Activity State
*/
GT_VOID prvTgfIpv4McRoutingAddManyConfigurationAndTrafficGenerateAndCheckActivityState()
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_LTT_ENTRY_STC                mcRouteLttEntry;
    GT_IPADDR                               ipGroup;
    GT_IPADDR                               ipSrc;
    TGF_MAC_ADDR                            daMac;
    TGF_MAC_ADDR                            saMac;
    GT_U32                                  i=0;
    GT_BOOL                                 activityBit;
    GT_BOOL                                 activityState;


    PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add IPv4 MC prefix 224.1.1.1/32 octet 3 is compress_1 bucket - already added*/
    /* AUTODOC: add IPv4 MC prefix 224.1.1.2/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.3/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.4/32 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.5/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.6/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.7/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.8/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.9/32 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.10/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.1.11/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.2.1/32 octet 2 is compress_1 bucket */
    /* AUTODOC: add IPv4 MC prefix 224.1.3.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.4.1/32 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 MC prefix 224.1.5.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.6.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.7.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.8.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.9.1/32 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add IPv4 MC prefix 224.1.10.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.1.11.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.2.1.1/32 octet 1 is compress_1 bucket */
    /* AUTODOC: add IPv4 MC prefix 224.3.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.4.1.1/32 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add IPv4 MC prefix 224.5.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.6.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.7.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.8.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.9.1.1/32 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add IPv4 MC prefix 224.10.1.1/32 */
    /* AUTODOC: add IPv4 MC prefix 224.11.1.1/32 */

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    prvTgfIpLpmActivityBitEnableGet(&activityBit);
    prvTgfIpLpmActivityBitEnableSet(GT_TRUE);

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */

    ipSrc.arIP[0]=1;
    ipSrc.arIP[1]=1;
    ipSrc.arIP[2]=1;
    ipSrc.arIP[3]=1;

    ipGroup.arIP[0]=224;
    ipGroup.arIP[1]=1;
    ipGroup.arIP[2]=1;
    ipGroup.arIP[3]=1;

    daMac[0]=1;
    daMac[1]=0;
    daMac[2]=0x5E;
    daMac[3]=1;
    daMac[4]=1;
    daMac[5]=1;

    saMac[0]=0;
    saMac[1]=0;
    saMac[2]=0;
    saMac[3]=0;
    saMac[4]=0;
    saMac[5]=0x33;

    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

    /* AUTODOC: check LPM Activity State */
    rc = prvTgfIpLpmIpv4McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 32, ipSrc, 32, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
            "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d ipSrc.u32Ip = %#x ipGroup.u32Ip = %#x", activityState, ipSrc.u32Ip, ipGroup.u32Ip);

    /* AUTODOC: add 29 IPv4 MC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipGroup.arIP[0]=224;
            ipGroup.arIP[1]=1;
            ipGroup.arIP[2]=1;
            ipGroup.arIP[3]=1;

            daMac[0]=1;
            daMac[1]=0;
            daMac[2]=0x5E;
            daMac[3]=1;
            daMac[4]=1;
            daMac[5]=1;
        }

        ipGroup.arIP[(((i-1)/10)+1)]++;
        daMac[(((i-1)/10)+3)]++;

        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 32,ipSrc,32, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

        rc = prvTgfIpLpmIpv4McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 32, ipSrc, 32, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
                "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d ipSrc.u32Ip = %#x ipGroup.u32Ip = %#x", activityState, ipSrc.u32Ip, ipGroup.u32Ip);
    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipGroup.arIP[0]=224;
            ipGroup.arIP[1]=1;
            ipGroup.arIP[2]=1;
            ipGroup.arIP[3]=1;

            daMac[0]=1;
            daMac[1]=0;
            daMac[2]=0x5E;
            daMac[3]=1;
            daMac[4]=1;
            daMac[5]=1;
        }

        ipGroup.arIP[(((i-1)/10)+1)]++;
        daMac[(((i-1)/10)+3)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

        rc = prvTgfIpLpmIpv4McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 32, ipSrc, 32, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
                "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d ipSrc.u32Ip = %#x ipGroup.u32Ip = %#x", activityState, ipSrc.u32Ip, ipGroup.u32Ip);
    }


 PRV_UTF_LOG0_MAC("==== Adding Prefixes ====\n");

    /* AUTODOC: add src IPv4 MC prefix 1.1.1.1/32 octet 3 is compress_1 bucket - already added*/
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.2/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.3/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.4/32 --> change bucket of octet 3 from compress_1 to compress_2 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.5/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.6/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.7/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.8/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.9/32 --> change bucket of octet 3 from compress_2 to regular */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.10/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.1.11/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.2.1/32 octet 2 is compress_1 bucket */
    /* AUTODOC: add src IPv4 MC prefix 1.1.3.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.4.1/32 --> change bucket of octet 2 from compress_1 to compress_2 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.5.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.6.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.7.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.8.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.9.1/32 --> change bucket of octet 2 from compress_2 to regular */
    /* AUTODOC: add src IPv4 MC prefix 1.1.10.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.1.11.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.2.1.1/32 octet 1 is compress_1 bucket */
    /* AUTODOC: add src IPv4 MC prefix 1.3.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.4.1.1/32 --> change bucket of octet 1 from compress_1 to compress_2 */
    /* AUTODOC: add src IPv4 MC prefix 1.5.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.6.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.7.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.8.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.9.1.1/32 --> change bucket of octet 1 from compress_2 to regular */
    /* AUTODOC: add src IPv4 MC prefix 1.10.1.1/32 */
    /* AUTODOC: add src IPv4 MC prefix 1.11.1.1/32 */

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a destination IP address for the prefix */

    ipSrc.arIP[0]=1;
    ipSrc.arIP[1]=1;
    ipSrc.arIP[2]=1;
    ipSrc.arIP[3]=1;

    ipGroup.arIP[0]=224;
    ipGroup.arIP[1]=1;
    ipGroup.arIP[2]=1;
    ipGroup.arIP[3]=1;

    daMac[0]=1;
    daMac[1]=0;
    daMac[2]=0x5E;
    daMac[3]=1;
    daMac[4]=1;
    daMac[5]=1;

    saMac[0]=0;
    saMac[1]=0;
    saMac[2]=0;
    saMac[3]=0;
    saMac[4]=0;
    saMac[5]=0x33;

    /* AUTODOC: send packet to the prefix added */
    prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

    rc = prvTgfIpLpmIpv4McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 32, ipSrc, 32, GT_TRUE, &activityState);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d", rc);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
            "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d ipSrc.u32Ip = %#x ipGroup.u32Ip = %#x", activityState, ipSrc.u32Ip, ipGroup.u32Ip);

    /* AUTODOC: add 29 IPv4 MC prefixes */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[0]=1;
            ipSrc.arIP[1]=1;
            ipSrc.arIP[2]=1;
            ipSrc.arIP[3]=1;

            saMac[0]=0;
            saMac[1]=0;
            saMac[2]=0;
            saMac[3]=0;
            saMac[4]=0;
            saMac[5]=1;
        }

        ipSrc.arIP[(((i-1)/10)+1)]++;
        saMac[(((i-1)/10)+3)]++;

        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, 32,ipSrc,32, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McPrefixAdd: %d", prvTgfDevNum);

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

        rc = prvTgfIpLpmIpv4McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 32, ipSrc, 32, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
                "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d ipSrc.u32Ip = %#x ipGroup.u32Ip = %#x", activityState, ipSrc.u32Ip, ipGroup.u32Ip);
    }

    /* AUTODOC: send packets to all prefixes added */
    for(i=30; i>0; i--)
    {
        if(i%10==0)
        {
            ipSrc.arIP[0]=1;
            ipSrc.arIP[1]=1;
            ipSrc.arIP[2]=1;
            ipSrc.arIP[3]=1;

            saMac[0]=0;
            saMac[1]=0;
            saMac[2]=0;
            saMac[3]=0;
            saMac[4]=0;
            saMac[5]=1;
        }

        ipSrc.arIP[(((i-1)/10)+1)]++;
        saMac[(((i-1)/10)+3)]++;

        /* AUTODOC: send packet to the prefix added */
        prvTgfIpv4McRoutingAddManyTrafficGenerate(ipGroup,ipSrc,daMac,saMac);

        rc = prvTgfIpLpmIpv4McEntryActivityStatusGet(prvTgfLpmDBId, 0, ipGroup, 32, ipSrc, 32, GT_TRUE, &activityState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d", rc);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, activityState,
                "prvTgfIpLpmIpv4McEntryActivityStatusGet: %d ipSrc.u32Ip = %#x ipGroup.u32Ip = %#x", activityState, ipSrc.u32Ip, ipGroup.u32Ip);
    }

    prvTgfIpLpmActivityBitEnableSet(activityBit);
}


/**
* @internal prvTgfIpv4McRoutingAddManyTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] dstIpAddr                - destination IP Address of the packet sent
* @param[in] srcIpAddr                - source IP Address of the packet sent
* @param[in] daMac                    - destination MAC address
* @param[in] saMac                    - source MAC address
*                                       None
*/
GT_VOID prvTgfIpv4McRoutingAddManyTrafficGenerate
(
    GT_IPADDR           dstIpAddr,
    GT_IPADDR           srcIpAddr,
    TGF_MAC_ADDR        daMac,
    TGF_MAC_ADDR        saMac
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

    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    /* set MLL counters mode for MC subscriber 1 */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
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
    cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, dstIpAddr.arIP, sizeof(prvTgfPacketIpv4Part.dstAddr));
    cpssOsMemCpy(prvTgfPacketIpv4Part.srcAddr, srcIpAddr.arIP, sizeof(prvTgfPacketIpv4Part.srcAddr));
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, saMac, sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, daMac, sizeof(prvTgfPacketL2Part.daMac));

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv4 packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=224.1.1.1 */
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
    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
       (isDualHwDevTest == GT_TRUE))
    {
        /* AUTODOC: verify 1 flood in ingress VLAN 80 to port 1 */
        prvTgfPacketsCountTxMC1 = 2;
    }

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


