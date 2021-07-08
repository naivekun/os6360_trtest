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
* @file prvTgfPclUdbL3Minus2.c
*
* @brief UDB Anchor L3 Mimus 2 Ingress and Eegress PCL testing
*
* @version   7
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfOamGen.h>
#include <pcl/prvTgfPclUdbL3Minus2.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS         2

/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             8

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0x55

/* original VLAN Id */
#define PRV_TGF_TAG0_VLAN_ID_CNS             0x10

/* new PCL VLAN Id */
#define PRV_TGF_PCL_NEW_VLAN_ID_CNS          0x31

/* new router VLAN Id */
#define PRV_TGF_ROUTER_NEW_VLAN_ID_CNS       0x42

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* ARP Entry index */
#define PRV_TGF_ARP_ENTRY_IDX_CNS            3

/* Router Source MAC_SA index  */
#define PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS  0x12

/* Router Source MAC_SA index  */
#define PRV_TGF_ROUTER_ENTRY_BASE_INDEX_CNS  0x16

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId   = 0;

/* backup configuration for restore after the test */
static GT_BOOL origRouterMacSaIndexValid = GT_FALSE;
static GT_U32  origRouterMacSaIndex = 0;
static GT_BOOL origMacSaAddrValid = GT_FALSE;
static GT_ETHERADDR origMacSaAddr    = {{0}};
/* PCL id for restore procedure */
static GT_U32 savePclId;
/********************************************************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part =
{
    {0x00, 0xFE, 0xDC, 0xBA, 0x98, 0x76},                /* daMac */
    {0x00, 0xBB, 0xAA, 0x11, 0x22, 0x33}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,    /* etherType */
    1, 0, PRV_TGF_TAG0_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketTunnelVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,    /* etherType */
    7, 0, PRV_TGF_TAG0_VLAN_ID_CNS     /* pri, cfi, VlanId */
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* ethertype part of MPLS packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart =
{
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4OtherHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    (GT_U8)(33 << 2),                                                /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)), /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    4,                                                               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    {22, 22, 22, 22},                                                /* srcAddr */
    { 1,  1,  1,  3}                                                 /* dstAddr */
};

/* MPLS Tunnel Header */
static TGF_PACKET_MPLS_STC prvTgfPacketTunnelMplsLabel0 =
{
    0x1111 /* Addressing Label */,
    0/*exp*/,
    1/*stack - last*/,
    0x55/*timeToLive*/
};

/* IPv4 Tunnel Without L2 in passegger */
static TGF_PACKET_IPV4_STC prvTgfPacketTunnelIpv4OtherHeaderNoL2Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x29,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    {16, 17, 18, 19},   /* srcAddr */
    { 4,  5,  6,  7}    /* dstAddr */
};

/* IPv4 Tunnel With L2 in passegger */
static TGF_PACKET_IPV4_STC prvTgfPacketTunnelIpv4OtherHeaderWithL2Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x30,               /* timeToLive */
    0x2A,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    {26, 27, 28, 29},   /* srcAddr */
    {14, 15, 16, 17}    /* dstAddr */
};

/* Tagged IPV4 Other Passenger Packet Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv4OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Tagged IPV4 Other Passenger Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv4OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),            /* totalLen */
    sizeof(prvTgfPacketPassengerIpv4OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv4OthPartsArray                                        /* partsArray */
};

/* Tagged IPV4 Other Tunnel Packet With L2 Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketTunnelIpv4OthWithL2PartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTunnelIpv4OtherHeaderWithL2Part},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Tagged IPV4 Other Tunnel Packet With L2 Parts Info */
static TGF_PACKET_STC prvTgfPacketTunnelIpv4OthWithL2Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),               /* totalLen */
    sizeof(prvTgfPacketTunnelIpv4OthWithL2PartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTunnelIpv4OthWithL2PartsArray                                        /* partsArray */
};

/* Tagged IPV4 Other Tunnel Packet Without L2 Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketTunnelIpv4OthNoL2PartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTunnelIpv4OtherHeaderNoL2Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Tagged IPV4 Other Tunnel Packet Without L2 Parts Info */
static TGF_PACKET_STC prvTgfPacketTunnelIpv4OthNoL2Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),             /* totalLen */
    sizeof(prvTgfPacketTunnelIpv4OthNoL2PartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTunnelIpv4OthNoL2PartsArray                                        /* partsArray */
};

/* Tagged MPLS Other Tunnel Packet With L2 Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketTunnelMplsWithL2PartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketTunnelMplsLabel0},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Tagged MPLS Other Tunnel Packet With L2 Parts Info */
static TGF_PACKET_STC prvTgfPacketTunnelMplsWithL2Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_MPLS_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),            /* totalLen */
    sizeof(prvTgfPacketTunnelMplsWithL2PartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTunnelMplsWithL2PartsArray                                        /* partsArray */
};

/* Tagged MPLS Other Tunnel Packet Without L2 Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketTunnelMplsNoL2PartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketTunnelMplsLabel0},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Tagged MPLS Tunnel Packet Without L2 Parts Info */
static TGF_PACKET_STC prvTgfPacketTunnelMplsNoL2Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_MPLS_HEADER_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),          /* totalLen */
    sizeof(prvTgfPacketTunnelMplsNoL2PartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTunnelMplsNoL2PartsArray                                        /* partsArray */
};

/********************************************************************/
static GT_U32   currentRuleIndex = 0;

static void setRuleIndex(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_U32  lookupId;

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        currentRuleIndex =  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_PCL_RULE_IDX_CNS); ;
    }
    else
    {
        lookupId = lookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                             0 ;/*CPSS_PCL_LOOKUP_0_E*/

        currentRuleIndex =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,PRV_TGF_PCL_RULE_IDX_CNS);
    }
}

/**
* @internal prvTgfPclUdbL3Minus2PortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfPclUdbL3Minus2PortVlanFdbSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in with all TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_TAG0_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as PCL assigned VID with all TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_NEW_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as PCL assigned VID with all TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_ROUTER_NEW_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketPassengerL2Part.daMac,
        PRV_TGF_TAG0_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketPassengerL2Part.daMac,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn Tunnel MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketTunnelL2Part.daMac,
        PRV_TGF_TAG0_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn Tunnel MAC_DA of with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketTunnelL2Part.daMac,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn Tunnel MAC_DA of with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketTunnelL2Part.daMac,
        PRV_TGF_ROUTER_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbL3Minus2PortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfPclUdbL3Minus2PortVlanFdbReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TAG0_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PCL_NEW_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ROUTER_NEW_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
}

/**
* @internal prvTgfPclUdbL3Minus2VidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
static GT_VOID prvTgfPclUdbL3Minus2VidTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal prvTgfPclUdbL3Minus2TrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
* @param[in] checkMsb                 - to check High bits of VID and UP
*                                       None
*/
static GT_VOID prvTgfPclUdbL3Minus2TrafficEgressVidCheck
(
    IN GT_U16  egressVid,
    IN GT_BOOL checkMsb
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    if (checkMsb == GT_FALSE)
    {
        vfdArray[0].offset = 15; /* 6 + 6 + 3 */
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = (GT_U8)(egressVid & 0xFF);
    }
    else
    {
        vfdArray[0].offset = 14; /* 6 + 6 + 2 */
        vfdArray[0].cycleCount = 2;
        vfdArray[0].patternPtr[0] = (GT_U8)((egressVid >> 8) & 0xFF);
        vfdArray[0].patternPtr[1] = (GT_U8)(egressVid & 0xFF);
    }

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfPclUdbL3Minus2PclLegacyCfgSet function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*                                       None
*/
static GT_STATUS prvTgfPclUdbL3Minus2PclLegacyCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormatArr[3];
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           portIndex;

    portIndex =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_RECEIVE_PORT_IDX_CNS
            : PRV_TGF_SEND_PORT_IDX_CNS;

    ruleFormatArr[0] =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    ruleFormatArr[1] =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    ruleFormatArr[2] = ruleFormatArr[1];

    ruleFormatArr[pktType] = ruleFormat;


    /* AUTODOC: Init and configure all needed PCL configuration per port Lookup0 */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[portIndex],
        direction, lookupNum,
        PRV_TGF_PCL_ID_CNS,
        ruleFormatArr[0] /*nonIpKey*/,
        ruleFormatArr[1] /*ipv4Key*/,
        ruleFormatArr[2] /*ipv6Key*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExt1");

    /* AUTODOC: action - TAG0 vlan modification */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.egressPolicy = GT_TRUE;
        action.vlan.egressVlanId0Cmd =
            PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }
    else
    {
        action.egressPolicy = GT_FALSE;
        action.vlan.vlanCmd =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }

    setRuleIndex(direction,lookupNum);

    /* AUTODOC: PCL rule: Write Rule */
    rc = prvTgfPclRuleSet(
        ruleFormat,
        currentRuleIndex,
        maskPtr, patternPtr, &action);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfPclUdbL3Minus2PclLegacyCfgReset function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
*                                       None
*/
static GT_STATUS prvTgfPclUdbL3Minus2PclLegacyCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize
)
{
    GT_STATUS rc,rc1 = GT_OK;

    /* AUTODOC: restore PCL configuration table entry */
    prvTgfPclPortsRestore(direction, lookupNum);

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
        ruleSize, currentRuleIndex, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdbL3Minus2PclUdbRangeCfgSet function
* @endinternal
*
* @brief   Set PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] offsetType               - offset Type
* @param[in] udbIndexBase             - udb Index Base
* @param[in] udbAmount                - udb Amount
* @param[in] udbOffsetBase            - udb Offset Base
* @param[in] udbOffsetIncrement       - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
static GT_STATUS prvTgfPclUdbL3Minus2PclUdbRangeCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbOffsetBase,
    IN GT_32                            udbOffsetIncrement
)
{
    GT_STATUS                            rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U8                                offset;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;
        offset = (GT_U8)(udbOffsetBase + (i * udbOffsetIncrement));

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction,udbIndex, offsetType, offset);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedByteSet");
    }

    return GT_OK;
}


/**
* @internal prvTgfPclUdbL3Minus2PclUdbRangeCfgReset function
* @endinternal
*
* @brief   Reset PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
*                                       None
*/
static GT_STATUS prvTgfPclUdbL3Minus2PclUdbRangeCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount
)
{
    GT_STATUS                            rc, rc1 = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction, udbIndex,
            PRV_TGF_PCL_OFFSET_INVALID_E, 0/*offset*/);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclUserDefinedByteSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdbL3Minus2RouterMacSaSet function
* @endinternal
*
* @brief   sets MA SA for packet with TS or TT with MAC SA Modify
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbL3Minus2RouterMacSaSet
(
    IN GT_VOID
)
{
    GT_STATUS rc;
    GT_ETHERADDR macSaAddr;

    origRouterMacSaIndexValid = GT_FALSE;
    origMacSaAddrValid        = GT_FALSE;

    /* AUTODOC: set MAC_SA of tunnel header */

    macSaAddr.arEther[0] =  prvTgfPacketTunnelL2Part.saMac[0];
    macSaAddr.arEther[1] =  prvTgfPacketTunnelL2Part.saMac[1];
    macSaAddr.arEther[2] =  prvTgfPacketTunnelL2Part.saMac[2];
    macSaAddr.arEther[3] =  prvTgfPacketTunnelL2Part.saMac[3];
    macSaAddr.arEther[4] =  prvTgfPacketTunnelL2Part.saMac[4];
    macSaAddr.arEther[5] =  prvTgfPacketTunnelL2Part.saMac[5];

    rc = prvTgfIpRouterPortGlobalMacSaIndexGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        &origRouterMacSaIndex);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

    origRouterMacSaIndexValid = GT_TRUE;

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaGet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &origMacSaAddr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterGlobalMacSaGet");

    origMacSaAddrValid = GT_TRUE;

    rc = prvTgfIpRouterGlobalMacSaSet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &macSaAddr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterGlobalMacSaSet");

    /* AUTODOC: set MAC SA Global mode for egress port */
    rc = prvTgfIpPortRouterMacSaLsbModeSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        CPSS_SA_LSB_FULL_48_BIT_GLOBAL);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "cpssDxChIpPortRouterMacSaLsbModeSet");

    return GT_OK;
}
/**
* @internal prvTgfPclUdbL3Minus2RouterMacSaReset function
* @endinternal
*
* @brief   resets MA SA for packet with TS or TT with MAC SA Modify
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbL3Minus2RouterMacSaReset
(
    IN GT_VOID
)
{
    GT_STATUS                            rc, rc1 = GT_OK;

    if (origRouterMacSaIndexValid == GT_TRUE)
    {
        rc = prvTgfIpRouterPortGlobalMacSaIndexSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            origRouterMacSaIndex);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");
    }

    if (origMacSaAddrValid == GT_TRUE)
    {
        rc = prvTgfIpRouterGlobalMacSaSet(
            prvTgfDevNum,
            PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
            &origMacSaAddr);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterGlobalMacSaSet");
    }

    /* AUTODOC: set MAC SA VLAN mode for egress port (default)*/
    rc = prvTgfIpPortRouterMacSaLsbModeSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        CPSS_SA_LSB_PER_VLAN_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpPortRouterMacSaLsbModeSet");

    origRouterMacSaIndexValid = GT_FALSE;
    origMacSaAddrValid        = GT_FALSE;

    return rc1;
}

/**
* @internal prvTgfPclUdbL3Minus2Ipv4RouterConfigSet function
* @endinternal
*
* @brief   Set IPV4 Router Configuration
*/
static GT_STATUS prvTgfPclUdbL3Minus2Ipv4RouterConfigSet
(
    IN GT_U16 srcVid,
    IN GT_U16 dstVid
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
     */

    cpssOsMemSet(&inlifEntry, 0, sizeof(inlifEntry));
    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_TRUE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;
    inlifEntry.vrfId             = 0;

    /* SCT Section - all zeros */

    /* Misc Section - all zeros */

    /* call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, 4096, &inlifEntry);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfInlifEntrySet");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port */
    rc = prvTgfIpPortRoutingEnable(
        PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
        CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(
        srcVid, CPSS_IP_UNICAST_E,
        CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpVlanRoutingEnable");

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */

    cpssOsMemCpy(
        arpMacAddr.arEther,
        prvTgfPacketTunnelL2Part.daMac,
        sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write a ARP MAC to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_ENTRY_IDX_CNS, &arpMacAddr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterArpAddrWrite");

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = dstVid;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = PRV_TGF_ARP_ENTRY_IDX_CNS;

    /* AUTODOC: add UC route entry with nexthop VLAN and nexthop port */
    rc = prvTgfIpUcRouteEntriesWrite(
        PRV_TGF_ROUTER_ENTRY_BASE_INDEX_CNS, routeEntriesArray, 1);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpUcRouteEntriesWrite");

    /* -------------------------------------------------------------------------
     * 4. AUTODOC: Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* AUTODOC: fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = PRV_TGF_ROUTER_ENTRY_BASE_INDEX_CNS;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: fill a destination IP address for the prefix */
    cpssOsMemCpy(
        ipAddr.arIP,
        prvTgfPacketPassengerIpv4OtherHeaderPart.dstAddr,
        sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix/32*/
    rc = prvTgfIpLpmIpv4UcPrefixAdd(
        prvTgfLpmDBId, 0 /*vrfId*/, ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpLpmIpv4UcPrefixAdd");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbL3Minus2Ipv4RouterConfigReset function
* @endinternal
*
* @brief   Reset IPV4 Router Configuration
*
* @param[in] srcVid                   - source vid
*                                       None
*/
static GT_STATUS prvTgfPclUdbL3Minus2Ipv4RouterConfigReset
(
    IN GT_U16 srcVid
)
{
    GT_STATUS                               rc, rc1 = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    GT_IPADDR                               ipAddr;

    PRV_UTF_LOG0_MAC("======= Resetting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Disable Routing
     */

    /* AUTODOC: disable Unicast IPv4 Routing on port */
    rc = prvTgfIpPortRoutingEnable(
        PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
        CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(
        srcVid, CPSS_IP_UNICAST_E,
        CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpVlanRoutingEnable");


    /* -------------------------------------------------------------------------
     * 2. Erase the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemSet(&arpMacAddr, 0, sizeof(arpMacAddr));

    /* AUTODOC: write a ARP MAC to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_ENTRY_IDX_CNS, &arpMacAddr);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterArpAddrWrite");

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    /* AUTODOC: add UC route entry */
    rc = prvTgfIpUcRouteEntriesWrite(
        PRV_TGF_ROUTER_ENTRY_BASE_INDEX_CNS, routeEntriesArray, 1);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpUcRouteEntriesWrite");

    /* -------------------------------------------------------------------------
     * 4. AUTODOC: Remove a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* AUTODOC: delete the Ipv4 prefix */
    /* AUTODOC: add IPv4 UC prefix/32*/
    /* AUTODOC: fill a destination IP address for the prefix */
    cpssOsMemCpy(
        ipAddr.arIP,
        prvTgfPacketPassengerIpv4OtherHeaderPart.dstAddr,
        sizeof(ipAddr.arIP));

    rc = prvTgfIpLpmIpv4UcPrefixDel(
        prvTgfLpmDBId, 0 /*vrfId*/, ipAddr, 32);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpLpmIpv4UcPrefixDel");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
    return GT_OK;
}

/**
* @internal prvTgfPclUdbL3Minus2TtiTunnelTermConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
* @param[in] passengerPacketType      - Passenger Packet Type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbL3Minus2TtiTunnelTermConfigSet
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType,
    IN PRV_TGF_TTI_PASSENGER_TYPE_ENT passengerPacketType
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));

    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, given passenger type */
    ttiAction.tunnelTerminate                   = GT_TRUE;
    ttiAction.ttPassengerPacketType             = passengerPacketType;
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    switch (tunnelPacketType)
    {
        case PRV_TGF_TTI_KEY_MPLS_E:
            ttiMask.mpls.common.pclId    = 0x3FF;
            ttiPattern.mpls.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_IPV4_E:
            ttiMask.ipv4.common.pclId    = 0x3FF;
            ttiPattern.ipv4.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        default: break;
    }

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, tunnelPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, tunnelPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_FALSE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclTunnelTermForceVlanModeEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbL3Minus2TtiTunnelTermConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbL3Minus2TtiTunnelTermConfigReset
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType
)
{
    GT_STATUS                rc, rc1 = GT_OK;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_FALSE);

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclTunnelTermForceVlanModeEnableSet");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfPclUdbL3Minus2TtiTunnelStartConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] originalPacketType       - original Packet Type (to enable lookup)
* @param[in] passengerPacketType      - Passenger Packet Type
* @param[in] tunnelType               - tunnel Type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbL3Minus2TtiTunnelStartConfigSet
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT          originalPacketType,
    IN PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT passengerPacketType,
    IN CPSS_TUNNEL_TYPE_ENT              tunnelType
)
{
    GT_STATUS                      rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC       ttiAction;
    PRV_TGF_TTI_RULE_UNT           ttiPattern;
    PRV_TGF_TTI_RULE_UNT           ttiMask;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTunnel/ARP Table entry */

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    switch (tunnelType)
    {
        case CPSS_TUNNEL_X_OVER_MPLS_E:
            tunnelEntry.mplsCfg.tagEnable    = GT_TRUE;
            tunnelEntry.mplsCfg.vlanId       = prvTgfPacketTunnelVlanTag0Part.vid;
            cpssOsMemCpy(
                tunnelEntry.mplsCfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
            tunnelEntry.mplsCfg.numLabels    = 1;
            tunnelEntry.mplsCfg.ttl          = prvTgfPacketTunnelMplsLabel0.timeToLive;
            tunnelEntry.mplsCfg.exp1         = prvTgfPacketTunnelMplsLabel0.exp;
            tunnelEntry.mplsCfg.label1       = prvTgfPacketTunnelMplsLabel0.label;
            break;
        case CPSS_TUNNEL_X_OVER_IPV4_E:
        case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        case CPSS_TUNNEL_GENERIC_IPV4_E:
            tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
            tunnelEntry.ipv4Cfg.vlanId           = prvTgfPacketTunnelVlanTag0Part.vid;
            tunnelEntry.ipv4Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
            tunnelEntry.ipv4Cfg.up               = 0;
            tunnelEntry.ipv4Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
            tunnelEntry.ipv4Cfg.dscp             = 0;
            tunnelEntry.ipv4Cfg.dontFragmentFlag = GT_FALSE;
            tunnelEntry.ipv4Cfg.ttl              = 33;
            tunnelEntry.ipv4Cfg.autoTunnel       = GT_FALSE;
            tunnelEntry.ipv4Cfg.autoTunnelOffset = 0;
            tunnelEntry.ipv4Cfg.ethType          = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
            tunnelEntry.ipv4Cfg.cfi              = 0;
            tunnelEntry.ipv4Cfg.retainCrc        = GT_FALSE;
            tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;

            /* tunnel next hop MAC DA, IP DA, IP SA */
            cpssOsMemCpy(
                tunnelEntry.ipv4Cfg.macDa.arEther,
                prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
            if (passengerPacketType == PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E)
            {
                /* passenger with L2 */
                cpssOsMemCpy(
                    tunnelEntry.ipv4Cfg.destIp.arIP,
                    prvTgfPacketTunnelIpv4OtherHeaderWithL2Part.dstAddr, sizeof(TGF_IPV4_ADDR));
                cpssOsMemCpy(
                    tunnelEntry.ipv4Cfg.srcIp.arIP,
                    prvTgfPacketTunnelIpv4OtherHeaderWithL2Part.srcAddr, sizeof(TGF_IPV4_ADDR));
            }
            else
            {
                /* passenger without L2 */
                cpssOsMemCpy(
                    tunnelEntry.ipv4Cfg.destIp.arIP,
                    prvTgfPacketTunnelIpv4OtherHeaderNoL2Part.dstAddr, sizeof(TGF_IPV4_ADDR));
                cpssOsMemCpy(
                    tunnelEntry.ipv4Cfg.srcIp.arIP,
                    prvTgfPacketTunnelIpv4OtherHeaderNoL2Part.srcAddr, sizeof(TGF_IPV4_ADDR));
            }
            break;
        default:
            rc = GT_BAD_PARAM;
            PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "not supported tunnel type");
            return rc;
    }

    rc = prvTgfTunnelStartEntrySet(
        PRV_TGF_ARP_ENTRY_IDX_CNS, tunnelType, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTunnelStartEntrySet");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, given passenger type */
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum    = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.bridgeBypass                      = GT_TRUE;
    ttiAction.tunnelStart                       = GT_TRUE;
    ttiAction.tsPassengerPacketType             = passengerPacketType;
    ttiAction.tunnelStartPtr                    = PRV_TGF_ARP_ENTRY_IDX_CNS;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.keepPreviousQoS                   = GT_TRUE;
    ttiAction.mplsCommand                       = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
    ttiAction.mplsLabel                         = prvTgfPacketTunnelMplsLabel0.label;
    ttiAction.mplsTtl                           = prvTgfPacketTunnelMplsLabel0.timeToLive;


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    switch (originalPacketType)
    {
        case PRV_TGF_TTI_KEY_MPLS_E:
            ttiMask.mpls.common.pclId    = 0x3FF;
            ttiPattern.mpls.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_IPV4_E:
            ttiMask.ipv4.common.pclId    = 0x3FF;
            ttiPattern.ipv4.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_ETH_E:
            ttiMask.eth.common.pclId    = 0x3FF;
            ttiPattern.eth.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_MIM_E:
            ttiMask.mim.common.pclId    = 0x3FF;
            ttiPattern.mim.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        default: break;
    }

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        originalPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, originalPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, originalPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, originalPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    rc = prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbL3Minus2TtiTunnelStartConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @param[in] originalPacketType       - origina Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbL3Minus2TtiTunnelStartConfigReset
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT          originalPacketType
)
{
    GT_STATUS                      rc, rc1 = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        originalPacketType, GT_FALSE);

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, originalPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    rc = prvTgfTunnelStartEntrySet(
        PRV_TGF_ARP_ENTRY_IDX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTunnelStartEntrySet");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfPclUdbL3Minus2TestGen function
* @endinternal
*
* @brief   General Test scenario
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] cfgPktType               - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ttiPacketType            - TTI Packet Type (to enable lookup)
* @param[in] ttPassengerPacketType    - Tunnel Termination Passenger Packet Type (For Ingress only)
* @param[in] tsPassengerPacketType    - Tunnel Start Passenger Packet Type (For Engress only)
* @param[in] tsTunnelType             - Tunnel Start  Tunnel Type (For Engress only)
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
* @param[in] patternArr[]             - array of 4 bytes - pattern for L3Minus2+(0-3)
*                                       None
*/
static GT_VOID prvTgfPclUdbL3Minus2TestGen
(
    IN CPSS_PCL_DIRECTION_ENT            direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT       packetType,
    IN GT_U32                            cfgPktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN PRV_TGF_TTI_KEY_TYPE_ENT          ttiPacketType,
    IN PRV_TGF_TTI_PASSENGER_TYPE_ENT    ttPassengerPacketType,
    IN PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT tsPassengerPacketType,
    IN CPSS_TUNNEL_TYPE_ENT              tsTunnelType,
    IN TGF_PACKET_STC                    *prvTgfPacketInfoPtr,
    IN GT_U8                             patternArr[]
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           udbIndexBase;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    GT_U32                           i;
    GT_BOOL                          toRoute;
    GT_BOOL                          toSetMacSa;
    GT_BOOL                          toTerminateTunnel;
    GT_BOOL                          toStartTunnel;
    GT_U16                           expectedVid;

    /* default */
    toRoute            = GT_FALSE;
    toSetMacSa         = GT_FALSE;
    toTerminateTunnel  = GT_FALSE;
    toStartTunnel      = GT_FALSE;
    expectedVid = PRV_TGF_PCL_NEW_VLAN_ID_CNS;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        toTerminateTunnel  = GT_TRUE;
        if ((ttPassengerPacketType != PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E)
            && (ttPassengerPacketType != PRV_TGF_TTI_PASSENGER_ETHERNET_CRC_E))
        {
            toRoute            = GT_TRUE;
            toSetMacSa         = GT_TRUE;
            expectedVid        = PRV_TGF_ROUTER_NEW_VLAN_ID_CNS;
        }
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
        udbIndexBase = 0;
        for (i = 0; (i < 4); i++)
        {
            mask.ruleExtNotIpv6.udb[i]    = 0xFF;
            pattern.ruleExtNotIpv6.udb[i] = patternArr[i];
        }
    }
    else
    {
        toStartTunnel      = GT_TRUE;
        toSetMacSa         = GT_TRUE;
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E;
        udbIndexBase = 12;
        for (i = 0; (i < 4); i++)
        {
            mask.ruleEgrExtNotIpv6.udb[i]    = 0xFF;
            pattern.ruleEgrExtNotIpv6.udb[i] = patternArr[i];
        }
    }

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbL3Minus2PortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    if (toTerminateTunnel == GT_TRUE)
    {
        /* AUTODOC: TTI configuration */
        rc = prvTgfPclUdbL3Minus2TtiTunnelTermConfigSet(
            ttiPacketType, ttPassengerPacketType);
        if (rc != GT_OK) goto label_restore;
    }

    if (toStartTunnel == GT_TRUE)
    {
        rc = prvTgfPclUdbL3Minus2TtiTunnelStartConfigSet(
            ttiPacketType, tsPassengerPacketType, tsTunnelType);
        if (rc != GT_OK) goto label_restore;
    }

    /* AUTODOC: PCL configuration */

    rc = prvTgfPclUdbL3Minus2PclLegacyCfgSet(
        direction,
        CPSS_PCL_LOOKUP_0_0_E,
        cfgPktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        ruleFormat, &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: UP0  - UDB30 L2 + 14,  mask 0xE0, patten - (up0 << 5)*/
    rc = prvTgfPclUdbL3Minus2PclUdbRangeCfgSet(
        direction,
        packetType,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E,
        udbIndexBase,
        4  /*udbAmount*/,
        0  /*udbOffsetBase*/,
        1  /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    if (toSetMacSa == GT_TRUE)
    {
        rc = prvTgfPclUdbL3Minus2RouterMacSaSet();
        if (rc != GT_OK) goto label_restore;
    }

    if (toRoute == GT_TRUE)
    {
        rc = prvTgfPclUdbL3Minus2Ipv4RouterConfigSet(
            PRV_TGF_PCL_NEW_VLAN_ID_CNS,
            PRV_TGF_ROUTER_NEW_VLAN_ID_CNS);
        if (rc != GT_OK) goto label_restore;
    }

    /* AUTODOC: Send Double Tagged IPV4 Other Packet */
    prvTgfPclUdbL3Minus2VidTrafficGenerate(prvTgfPacketInfoPtr);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbL3Minus2TrafficEgressVidCheck(expectedVid, GT_FALSE/*check 8 bit*/);

    label_restore:

    if (toRoute == GT_TRUE)
    {
        rc = prvTgfPclUdbL3Minus2Ipv4RouterConfigReset(PRV_TGF_ROUTER_NEW_VLAN_ID_CNS);
        if (rc != GT_OK) goto label_restore;
    }

    if (toSetMacSa == GT_TRUE)
    {
        rc = prvTgfPclUdbL3Minus2RouterMacSaReset();
        if (rc != GT_OK) goto label_restore;
    }

    prvTgfPclUdbL3Minus2PclLegacyCfgReset(
        direction,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdbL3Minus2PclUdbRangeCfgReset(
        direction,
        packetType,
        udbIndexBase,
        4  /*udbAmount*/);

    if (toStartTunnel == GT_TRUE)
    {
        prvTgfPclUdbL3Minus2TtiTunnelStartConfigReset(ttiPacketType);
    }

    if (toTerminateTunnel == GT_TRUE)
    {
        prvTgfPclUdbL3Minus2TtiTunnelTermConfigReset(ttiPacketType);
    }

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbL3Minus2PortVlanFdbReset();

}

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Ipv4 With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4WithL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];

    patternArr[0] = (GT_U8)((prvTgfPacketIpv4EtherTypePart.etherType >> 8) & 0xFF);
    patternArr[1] = (GT_U8)(prvTgfPacketIpv4EtherTypePart.etherType & 0xFF);
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_IPV4_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E   /*ttPassengerPacketType*/,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E /*tsPassengerPacketType*/ /*not relevant*/,
        CPSS_TUNNEL_GENERIC_IPV4_E /*tsTunnelType*/ /*not relevant*/,
        &prvTgfPacketTunnelIpv4OthWithL2Info,
        patternArr);
}

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Ipv4 Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverIpv4NoL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];

    /* when passenger has no L2               */
    /* L3Minus2+0, L3Minus2+1 invalid - zeros */
    patternArr[0] = 0 /*offsert=0 - invalid*/;
    patternArr[1] = 0 /*offsert=1 - invalid*/;
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_IPV4_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_IPV4_E   /*ttPassengerPacketType*/,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E /*tsPassengerPacketType*/ /*not relevant*/,
        CPSS_TUNNEL_GENERIC_IPV4_E /*tsTunnelType*/ /*not relevant*/,
        &prvTgfPacketTunnelIpv4OthNoL2Info,
        patternArr);
}

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Mpls With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsWithL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];
    patternArr[0] = (GT_U8)((prvTgfPacketIpv4EtherTypePart.etherType >> 8) & 0xFF);
    patternArr[1] = (GT_U8)(prvTgfPacketIpv4EtherTypePart.etherType & 0xFF);
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_MPLS_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E   /*ttPassengerPacketType*/,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E /*tsPassengerPacketType*/ /*not relevant*/,
        CPSS_TUNNEL_GENERIC_IPV4_E /*tsTunnelType*/ /*not relevant*/,
        &prvTgfPacketTunnelMplsWithL2Info,
        patternArr);
}

/**
* @internal prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2Test function
* @endinternal
*
* @brief   Test on IPCL L3Minus2 anchor for Tunnel Terminated Ipv4 Over Mpls Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2IngressTunnelTermIpv4OverMplsNoL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];

    /* when passenger has no L2               */
    /* L3Minus2+0, L3Minus2+1 invalid - zeros */
    patternArr[0] = 0 /*offsert=0 - invalid*/;
    patternArr[1] = 0 /*offsert=1 - invalid*/;
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_MPLS_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_IPV4_E   /*ttPassengerPacketType*/,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E /*tsPassengerPacketType*/ /*not relevant*/,
        CPSS_TUNNEL_GENERIC_IPV4_E /*tsTunnelType*/ /*not relevant*/,
        &prvTgfPacketTunnelMplsNoL2Info,
        patternArr);
}

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Ipv4 With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4WithL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];

    patternArr[0] = (GT_U8)((prvTgfPacketIpv4EtherTypePart.etherType >> 8) & 0xFF);
    patternArr[1] = (GT_U8)(prvTgfPacketIpv4EtherTypePart.etherType & 0xFF);
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_IPV4_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E   /*ttPassengerPacketType*/ /*not relevant*/,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E /*tsPassengerPacketType*/,
        CPSS_TUNNEL_GENERIC_IPV4_E /*tsTunnelType*/,
        &prvTgfPacketPassengerIpv4OthInfo,
        patternArr);
}

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Ipv4 Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverIpv4NoL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];

    /* when passenger has no L2               */
    /* L3Minus2+0, L3Minus2+1 invalid - zeros */
    patternArr[0] = 0; /* invalid */
    patternArr[1] = 0; /* invalid */
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_IPV4_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E   /*ttPassengerPacketType*/ /*not relevant*/,
        PRV_TGF_TUNNEL_PASSENGER_OTHER_E /*tsPassengerPacketType*/,
        CPSS_TUNNEL_GENERIC_IPV4_E /*tsTunnelType*/,
        &prvTgfPacketPassengerIpv4OthInfo,
        patternArr);
}

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Mpls With Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsWithL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];

    patternArr[0] = (GT_U8)((prvTgfPacketIpv4EtherTypePart.etherType >> 8) & 0xFF);
    patternArr[1] = (GT_U8)(prvTgfPacketIpv4EtherTypePart.etherType & 0xFF);
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_IPV4_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E   /*ttPassengerPacketType*/ /*not relevant*/,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E /*tsPassengerPacketType*/,
        CPSS_TUNNEL_X_OVER_MPLS_E /*tsTunnelType*/,
        &prvTgfPacketPassengerIpv4OthInfo,
        patternArr);
}

/**
* @internal prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2Test function
* @endinternal
*
* @brief   Test on EPCL L3Minus2 anchor for Tunnel Start Ipv4 Over Mpls Without Passenger L2
*         UDBs for anchor L3Minus2 offset 0-3.
*/
GT_VOID prvTgfPclUdbL3Minus2EgressTunnelStartIpv4OverMplsNoL2Test
(
    GT_VOID
)
{
    GT_U8   patternArr[4];

    /* when passenger has no L2               */
    /* L3Minus2+0, L3Minus2+1 invalid - zeros */
    patternArr[0] = 0; /* invalid */
    patternArr[1] = 0; /* invalid */
    patternArr[2] =
        (GT_U8)((prvTgfPacketPassengerIpv4OtherHeaderPart.version << 4)
         | prvTgfPacketPassengerIpv4OtherHeaderPart.headerLen);
    patternArr[3] = (GT_U8)prvTgfPacketPassengerIpv4OtherHeaderPart.typeOfService;

    prvTgfPclUdbL3Minus2TestGen(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        1 /*cfgPktType*/, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_TTI_KEY_IPV4_E /*ttiPacketType*/,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E   /*ttPassengerPacketType*/ /*not relevant*/,
        PRV_TGF_TUNNEL_PASSENGER_OTHER_E /*tsPassengerPacketType*/,
        CPSS_TUNNEL_X_OVER_MPLS_E /*tsTunnelType*/,
        &prvTgfPacketPassengerIpv4OthInfo,
        patternArr);
}



