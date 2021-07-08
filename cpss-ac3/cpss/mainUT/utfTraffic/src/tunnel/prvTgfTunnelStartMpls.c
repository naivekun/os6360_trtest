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
* @file prvTgfTunnelStartMpls.c
*
* @brief Tunnel Start: Mpls functionality
*
* @version   15
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
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelStartMpls.h>
#include <common/tgfCosGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS              1

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5
/* egress VLAN1 Id */
#define PRV_TGF_VLANID1_CNS              0x333
/* egress MPLS TS header Vlan Id */
#define PRV_TGF_MPLS_TS_VLANID_CNS       0xaaa

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* control word index to set  */
#define PRV_TGF_CONTRL_WORD_IDX_CNS       1

/* control word value to set  */
#define PRV_TGF_CONTRL_WORD_VALUE_CNS     0x00CDEF98

/* control word index to set  */
#define PRV_TGF_CONTRL_WORD_VALUE_SIZE_CNS 4

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

#define PRV_TGF_EPCL_RULE_INDEX_CNS  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(900)
#define PRV_TGF_EPCL_PCL_ID_CNS      1000
#define PRV_TGF_EPCL_OUTER_TAG_MODIFIED_VID_CNS  0xABC
#define PRV_TGF_EPCL_OUTER_TAG_MODIFIED_UP_CNS   4


/************************ common parts of packets **************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};
/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x32,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/************************* packet 1 (IPv4 packet) **************************/

/* PARTS of packet 1 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET 1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    ( TGF_L2_HEADER_SIZE_CNS
    + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
    + sizeof(prvTgfPacket1PayloadDataArr)),                       /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/**************************** packet MPLS **********************************/
#define ORIG_OUTER_MPLS_EXP_CNS   7
#define OUTER_MPLS_EXP_AFTER_EXP_REMAP_CNS   2
#define BASIC_EXP_CNS 0
/* TTL for all */
#define TS_TTL_FOR_LABELS_CNS   0xEE

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketMplsVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* MPLS ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsMplsEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};
/* MPLS part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMplsPart = {
    0x66,               /* label */
    ORIG_OUTER_MPLS_EXP_CNS,      /* exp */
    1,                  /* stack */
    0xFF                /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfPacketMplsPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketMplsVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMplsPart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET MPLS to send */
static TGF_PACKET_STC prvTgfPacketMplsInfo = {
    ( TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
    + TGF_ETHERTYPE_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS
    + TGF_IPV4_HEADER_SIZE_CNS
    + sizeof(prvTgfPacket1PayloadDataArr)),                          /* totalLen */
    sizeof(prvTgfPacketMplsPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMplsPartArray                                        /* partsArray */
};

/********************* packet MPLS with 3 labels ***************************/
/* MPLS 3 part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMpls3Part = {
    0x33,               /* label */
    ORIG_OUTER_MPLS_EXP_CNS,      /* exp */
    0,                  /* stack */
    0xdd                /* timeToLive */
};
/* MPLS 2 part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMpls2Part = {
    0x22,               /* label */
    6,                  /* exp */
    0,                  /* stack */
    0xee                /* timeToLive */
};
/* MPLS 1 part */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsMpls1Part = {
    0x11,               /* label */
    5,                  /* exp */
    1,                  /* stack */
    0xFF                /* timeToLive */
};

/* PARTS of packet MPLS 3 */
static TGF_PACKET_PART_STC prvTgfPacketMpls3PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketMplsVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls3Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls2Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls1Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};
/* PACKET MPLS 3 to send */
static TGF_PACKET_STC prvTgfPacketMpls3Info = {
    ( TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
    + TGF_ETHERTYPE_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS
    + TGF_MPLS_HEADER_SIZE_CNS + TGF_MPLS_HEADER_SIZE_CNS
    + TGF_IPV4_HEADER_SIZE_CNS
    + sizeof(prvTgfPacket1PayloadDataArr)),                           /* totalLen */
    sizeof(prvTgfPacketMpls3PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMpls3PartArray                                        /* partsArray */
};

/* Ethernet packet (non IP)*/
/* ethertype part of ethernet packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthernetEtherTypePart = {
    0x6666
};

/* DATA of ethernet packet */
static GT_U8 prvTgfPacketEthernetPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb, 0xaa, 0xbb,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketEthernetPayloadPart = {
    sizeof(prvTgfPacketEthernetPayloadDataArr),                 /* dataLength */
    prvTgfPacketEthernetPayloadDataArr                          /* dataPtr */
};

/* ethernet packet : tag 0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthernetTag0VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* ethernet packet : tag 1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthernetTag1VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID1_CNS                           /* pri, cfi, VlanId */
};

/* PARTS of packet ethernet */
static TGF_PACKET_PART_STC prvTgfPacketEthernetPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketEthernetTag0VlanTagPart},/* ingress only tag 0 */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthernetEtherTypePart},/*ethernet packet -- non IP*/
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketEthernetPayloadPart}
};
/* PACKET Ethernet to send */
static TGF_PACKET_STC prvTgfPacketEthernetInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                       /* totalLen */
    sizeof(prvTgfPacketEthernetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthernetPartArray                                        /* partsArray */
};

/* MPLS TS tag part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketTunnelStartMplsVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_MPLS_TS_VLANID_CNS                    /* pri, cfi, VlanId */
};

/* MPLS TS : L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketTunnelStartMplsL2Part = {
    {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee},                /* daMac */
    {0x00, 0x99, 0x88, 0x77, 0x66, 0x55}                 /* saMac */
};

/* PARTS of MPLS TS with the passenger of packet ethernet */
static TGF_PACKET_PART_STC prvTgfPacketExpectedTsMplsPassengerEthernetPartArray[] = {
    /* the TS parts */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelStartMplsL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelStartMplsVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls3Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls2Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsMpls1Part},

    /* the passenger part (note the tag0,tag1 order)*/
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketEthernetTag1VlanTagPart},/* outer tag 1 */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketEthernetTag0VlanTagPart},/* inner tag 0 */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthernetEtherTypePart},/*ethernet packet -- non IP*/
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketEthernetPayloadPart}
};

/* the expected MPLS TS with the passenger of packet Ethernet to receive  */
static TGF_PACKET_STC prvTgfPacketExpectedTsMplsPassengerEthernetInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                       /* totalLen */
    sizeof(prvTgfPacketExpectedTsMplsPassengerEthernetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketExpectedTsMplsPassengerEthernetPartArray                                        /* partsArray */
};
/* orig vid1 of the ingress port */
static GT_U16   origVid1 = 0;
static PRV_TGF_BRG_EGRESS_PORT_INFO_STC    origEgressInfo = {0};
static GT_U32  testRouterMacSaIndex = 53;
static GT_U32   origRouterMacSaIndex = 0;
static GT_ETHERADDR origMacSaAddr = {{0}};
static TGF_TTL origTtl1=0,origTtl2=0,origTtl3=0;



/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfStartEntryMac = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x11
};

static const GT_CHAR prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "sending packet to port [%d] ...\n"
};
static TGF_PACKET_STC *prvTgfPacketInfoArray[] = {
    &prvTgfPacket1Info
};

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/* PCL Rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* TTI Rule index */
static GT_U32   prvTgfTtiRuleIndex = 1;

/* the Arp Address index of the Router ARP Table
 * to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32   prvTgfRouterArpIndex = 1;

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32   prvTgfTunnelStartLineIndex = 8;

/* define expected number of received packets (Sent packets) */
static GT_U8 prvTgfPacketsCountRxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports:  SEND VLN1 VLN2 RECEIVE */
             {1,  0,  0,  1}
};

/* define expected number of transmitted packets */
static GT_U8 prvTgfPacketsCountTxArray[][PRV_TGF_PORTS_NUM_CNS] = {
/* ports:  SEND VLN1 VLN2 RECEIVE */
             {1,  0,  0,  1}
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT        macMode;
    GT_U32                          controlWordvalue;
} prvTgfRestoreCfg;

/* egress QoS data */
static GT_BOOL egressQosExpEnableGet;
static GT_U32 egressMappingTableIndexGet;
static GT_U32 exp2expGet;
/******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCountersIpReset function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCountersIpReset
(
    GT_VOID
)
{
    GT_U32       portIter;
    GT_STATUS rc = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset IP couters and set CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E mode */
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersIpSet");
    }

    return rc;
};

/**
* @internal prvTgfCountersIpCheck function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfCountersIpCheck
(
    GT_VOID
)
{
    GT_STATUS                  rc = GT_OK;
    GT_U32                     portIter;
    PRV_TGF_IP_COUNTER_SET_STC ipCounters;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);

        /* check IP counters */
        rc = prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersIpGet");
    }

    PRV_UTF_LOG0_MAC("\n");

    return rc;
};

/**
* @internal prvTgfPortCaptureEnable function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPortCaptureEnable
(
    IN GT_U32   portNum,
    IN GT_BOOL start
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, start);
    PRV_UTF_VERIFY_GT_OK(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    return rc;
};

/**
* @internal prvTgfPortCapturePacketPrint function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfPortCapturePacketPrint
(
    IN GT_U32        portNum,
    IN TGF_MAC_ADDR macPattern,
    OUT GT_U32     *numTriggersPtr
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    CPSS_NULL_PTR_CHECK_MAC(numTriggersPtr);
    *numTriggersPtr = 0;

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNum);

    /* set nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set pattern as MAC to select captured packets */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, macPattern, sizeof(TGF_MAC_ADDR));

    /* check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, numTriggersPtr);
    rc = rc == GT_NO_MORE ? GT_OK : rc;

    return rc;
};

/**
* @internal prvTgfVlanSet function
* @endinternal
*
* @brief   Set VLAN entry
*
* @param[in] vlanId                   -  to be configured
* @param[in] portsMembers             - VLAN's ports Members
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfVlanSet
(
    IN GT_U16          vlanId,
    IN CPSS_PORTS_BMP_STC portsMembers
)
{
    GT_STATUS                          rc = GT_OK;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    CPSS_PORTS_BMP_STC                 portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC          vlanInfo;

    /* clear ports tagging */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    vlanInfo.autoLearnDisable = GT_TRUE; /* working in controlled learning */
    vlanInfo.naMsgToCpuEn     = GT_TRUE; /* working in controlled learning */

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanEntryWrite");

    return rc;
};

/**
* @internal prvTgfDefBrigeConfSet function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefBrigeConfSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PORTS_BMP_STC              vlanMembers = {{0, 0}};

    /* clear entry */
    cpssOsMemSet(&vlanMembers, 0, sizeof(vlanMembers));

    /* ports 0, 8 are VLAN_1 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS]);

    prvTgfVlanSet(PRV_TGF_SEND_VLANID_CNS, vlanMembers);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfVlanSet");

    /* clear entry */
    cpssOsMemSet(&vlanMembers, 0, sizeof(vlanMembers));

    /* ports 18, 23 are VLAN_2 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&vlanMembers,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    prvTgfVlanSet(PRV_TGF_NEXTHOPE_VLANID_CNS, vlanMembers);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfVlanSet");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidGet");

    /* set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* create a macEntry for original packet 1 in VLAN 5 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_SEND_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* create a macEntry for tunneled packet 1 in VLAN 6 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_NEXTHOPE_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
};

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfConfigurationRestore
(
    IN GT_BOOL isPclSet,
    IN GT_BOOL isTtiSet
)
{
    GT_STATUS rc = GT_OK;

    /* invalidate PCL rules */
    if (isPclSet)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfPclRuleValidStatusSet");
        prvTgfPclRestore();
    }

    /* invalidate TTI rules */
    if (isTtiSet)
    {
        rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiRuleValidStatusSet");
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");

    /* restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc;
};

/**
* @internal prvTgfTrafficGenerate function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    sendIter;

    /* sending packets */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC(
            prvTgfIterationNameArray[sendIter],
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, prvTgfPacketInfoArray[sendIter],
            1 /*burstCount*/, 0 /*numVfd*/, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }

    return rc;
};

/**
* @internal prvTgfTtiEnable function
* @endinternal
*
* @brief   This function enables the TTI lookup
*         and sets the lookup Mac mode
*         for the specified key type
* @param[in] portNum                  - port number
* @param[in] ttiKeyType               - TTI key type
* @param[in] macMode                  - MAC mode to use
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfTtiEnable
(
    IN  GT_U32                    portNum,
    IN  PRV_TGF_TTI_KEY_TYPE_ENT ttiKeyType,
    IN  PRV_TGF_TTI_MAC_MODE_ENT macMode,
    IN  GT_BOOL                  isTtiEnable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ethType;

    if (GT_TRUE == isTtiEnable)
    {
        /* set MIM EtherType */
        ethType = TGF_ETHERTYPE_88E7_MIM_TAG_CNS;

        /* save current Eth TTI key lookup MAC mode */
        rc = prvTgfTtiMacModeGet(prvTgfDevNum, ttiKeyType, &prvTgfRestoreCfg.macMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeGet");
    }
    else
    {
        /* restore default MIM EtherType */
        ethType = TGF_ETHERTYPE_88E7_MIM_TAG_CNS;

        /* restore Eth TTI key lookup MAC mode */
        macMode = prvTgfRestoreCfg.macMode;
    }

    /* set MIM EtherType */
    rc = prvTgfTtiMimEthTypeSet(ethType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMimEthTypeSet");

    /* enable/disable port for ttiKeyType TTI lookup */
    rc = prvTgfTtiPortLookupEnableSet(portNum, ttiKeyType, isTtiEnable);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* set TTI lookup MAC mode */
    rc = prvTgfTtiMacModeSet(ttiKeyType, macMode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiMacModeSet");

    return rc;
};


/**
* @internal prvTgfDefTtiMplsPushRuleSet function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefTtiMplsPushRuleSet
(
    IN PRV_TGF_TUNNEL_MPLS_LSR_PUSH_TYPE_ENT pushType
)
{
    GT_STATUS              rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT   ttiPattern;
    PRV_TGF_TTI_RULE_UNT   ttiMask;

    /* cpssDxChTtiRuleSet(CPSS_DXCH_TTI_MPLS_RULE_STC, CPSS_DXCH_TTI_ACTION_2_STC) */

    /* clear action */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    /* set action MPLS command */
    switch(pushType) {
    case PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E:
        ttiAction.mplsCmd = PRV_TGF_TTI_MPLS_SWAP_COMMAND_E;
        break;
    case PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E:
        ttiAction.mplsCmd = PRV_TGF_TTI_MPLS_POP_AND_SWAP_COMMAND_E;
        break;
    default:
        ttiAction.mplsCmd = PRV_TGF_TTI_MPLS_PUSH1_LABEL_COMMAND_E;
        break;
    }

    /* set action - redirect to port 23 */
    ttiAction.tunnelTerminate               = GT_FALSE;
    ttiAction.passengerPacketType           = PRV_TGF_TTI_PASSENGER_MPLS_E;
    ttiAction.tsPassengerPacketType         = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.tag0VlanCmd                   = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                    = 6;
    ttiAction.tag1VlanCmd                   = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
    ttiAction.tag1VlanId                    = 0;
    ttiAction.keepPreviousQoS               = GT_TRUE;
    ttiAction.tunnelStart                   = GT_TRUE;
    ttiAction.tunnelStartPtr                = prvTgfTunnelStartLineIndex;

    /* mask and pattern - mpls labels */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(ttiMask.mpls.common.mac.arEther, 0xFF, sizeof(TGF_MAC_ADDR));
    ttiMask.mpls.common.vid         = 0x0F;  /* 4095 (0xFFF) in RDE test */
    ttiMask.mpls.common.pclId       = 0x3FF; /* 4095 (0xFFF) in RDE test */
    ttiMask.mpls.numOfLabels        = 0xFF;  /* 3 in RDE test */
    ttiMask.mpls.label0             = 0xFF;  /* 1048575 (0xFFFFF) in RDE test */
    ttiMask.mpls.common.isTagged    = GT_TRUE;
    ttiMask.mpls.exp0               = 7;

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemCpy(ttiPattern.mpls.common.mac.arEther, prvTgfPacket1L2Part.daMac, sizeof(TGF_MAC_ADDR));
    ttiPattern.mpls.common.vid      = PRV_TGF_SEND_VLANID_CNS;
    ttiPattern.mpls.common.pclId    = 2;
    ttiPattern.mpls.common.isTagged = GT_TRUE;
    ttiPattern.mpls.exp0            = ORIG_OUTER_MPLS_EXP_CNS;

    /* set labels */
    switch(pushType) {
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E:
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_2_LABEL_E:
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_3_LABEL_E:
        ttiPattern.mpls.numOfLabels     = 0;
        ttiPattern.mpls.label0          = 0x66;
        break;
    case PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E:
    case PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E:
        ttiPattern.mpls.numOfLabels     = 2;
        ttiMask.mpls.label1             = 0xFF;
        ttiMask.mpls.label2             = 0xFF;
        ttiPattern.mpls.label0          = 0x33;
        ttiPattern.mpls.label1          = 0x22;
        ttiPattern.mpls.label2          = 0x11;
        break;
    default:
        break;
    }

    /* set TTI Rule */
    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_MPLS_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleSet");

    /* debug - Check TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    rc = prvTgfTtiRuleGet(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_MPLS_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleGet");

    return rc;
};

/**
* @internal prvTgfDefTtiMplsPopRuleSet function
* @endinternal
*
* @brief   None
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefTtiMplsPopRuleSet
(
    IN PRV_TGF_TUNNEL_MPLS_LSR_POP_TYPE_ENT popType
)
{
    GT_STATUS              rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT   ttiPattern;
    PRV_TGF_TTI_RULE_UNT   ttiMask;

    /* cpssDxChTtiRuleSet(CPSS_DXCH_TTI_MPLS_RULE_STC, CPSS_DXCH_TTI_ACTION_2_STC) */

    /* clear action */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    /* set action MPLS command */
    if(PRV_TGF_TUNNEL_MPLS_LSR_POP_2_LABEL_E == popType)
        ttiAction.mplsCmd                   = PRV_TGF_TTI_MPLS_POP2_LABELS_COMMAND_E;
    else
        ttiAction.mplsCmd                   = PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E;

    /* set action - redirect to port 23 */
    ttiAction.tunnelTerminate               = GT_FALSE;
    ttiAction.passengerPacketType           = PRV_TGF_TTI_PASSENGER_MPLS_E;
    ttiAction.tsPassengerPacketType         = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;
    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.tag0VlanCmd                   = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                    = 6;
    ttiAction.tag1VlanCmd                   = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
    ttiAction.tag1VlanId                    = 0;
    ttiAction.keepPreviousQoS               = GT_TRUE;
    ttiAction.tunnelStart                   = GT_FALSE;
    ttiAction.tunnelStartPtr                = 0;
    ttiAction.arpPtr                        = prvTgfRouterArpIndex;

    /* mask and pattern - mpls labels */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(ttiMask.mpls.common.mac.arEther, 0xFF, sizeof(TGF_MAC_ADDR));
    ttiMask.mpls.common.vid         = 0x0F;
    ttiMask.mpls.label0             = 0xFF;
    ttiMask.mpls.label1             = 0xFF;
    ttiMask.mpls.label2             = 0xFF;
    ttiMask.mpls.numOfLabels        = 0xFF;
    ttiMask.mpls.common.pclId       = 0x3FF;

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemCpy(ttiPattern.mpls.common.mac.arEther, prvTgfPacket1L2Part.daMac, sizeof(TGF_MAC_ADDR));
    ttiPattern.mpls.common.vid      = PRV_TGF_SEND_VLANID_CNS;
    ttiPattern.mpls.label0          = 0x33;
    ttiPattern.mpls.label1          = 0x22;
    ttiPattern.mpls.label2          = 0x11;
    ttiPattern.mpls.numOfLabels     = 2;
    ttiPattern.mpls.common.pclId    = 2;

    /* set TTI Rule */
    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_MPLS_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleSet");

    /* debug - Check TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    rc = prvTgfTtiRuleGet(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_MPLS_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTtiRuleGet");

    return rc;
};

/**
* @internal prvTgfDefTunnelStartEntryMplsSet function
* @endinternal
*
* @brief   Set a default X-over-MPLS tunnel start entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefTunnelStartEntryMplsSet
(
    IN PRV_TGF_TUNNEL_MPLS_LSR_PUSH_TYPE_ENT pushType
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    /* set a Tunnel Start Entry
     * cpssDxChTunnelStartEntrySet(,,CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC)
     */
    tunnelType = CPSS_TUNNEL_X_OVER_MPLS_E; /* eliminate Coverity warning */

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.mplsCfg.tagEnable        = GT_TRUE;
    tunnelEntry.mplsCfg.vlanId           = PRV_TGF_NEXTHOPE_VLANID_CNS;
    cpssOsMemCpy(tunnelEntry.mplsCfg.macDa.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR));
    tunnelEntry.mplsCfg.ttl              = TS_TTL_FOR_LABELS_CNS;
    tunnelEntry.mplsCfg.exp1             = 1;

    /* set labels */
    switch(pushType) {
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E:
        tunnelEntry.mplsCfg.numLabels    = 1;
        tunnelEntry.mplsCfg.label1       = 0x11;
        break;
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_2_LABEL_E:
        tunnelEntry.mplsCfg.numLabels    = 2;
        tunnelEntry.mplsCfg.label1       = 0x11;
        tunnelEntry.mplsCfg.label2       = 0x22;
        break;
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_3_LABEL_E:
        tunnelEntry.mplsCfg.numLabels    = 3;
        tunnelEntry.mplsCfg.label1       = 0x11;
        tunnelEntry.mplsCfg.label2       = 0x22;
        tunnelEntry.mplsCfg.label3       = 0x33;
        break;
    case PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E:
    case PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E:
        tunnelEntry.mplsCfg.numLabels    = 1;
        tunnelEntry.mplsCfg.label1       = 0x66;
        break;
    default:
        break;
    }

    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex, CPSS_TUNNEL_X_OVER_MPLS_E, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntrySet");

    /* debug - check Tunnel Start Entry - vlanId and macDa fields */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfTunnelStartLineIndex, &tunnelType, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    /* check tunnelType */
    rc = (CPSS_TUNNEL_X_OVER_MPLS_E == tunnelType)? GT_OK : GT_FAIL;
    if (GT_OK != rc)
        PRV_UTF_LOG1_MAC("tunnelType: %d\n", tunnelType);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    /* check VLAN Id field */
    rc = (PRV_TGF_NEXTHOPE_VLANID_CNS == tunnelEntry.mplsCfg.vlanId)? GT_OK : GT_FAIL;
    if (GT_OK != rc)
        PRV_UTF_LOG1_MAC("tunnelEntry.mplsCfg.vlanId: %d\n", tunnelEntry.mplsCfg.vlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    /* check MAC DA field */
    rc = cpssOsMemCmp(tunnelEntry.mplsCfg.macDa.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FAIL;
    if (GT_OK != rc)
        PRV_UTF_LOG1_MAC("tunnelEntry.mplsCfg.macDa.arEther[5]: 0x%2X\n", tunnelEntry.mplsCfg.macDa.arEther[5]);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntryGet");

    return rc;
};

/**
* @internal prvTgfDefArpEntrySet function
* @endinternal
*
* @brief   Set a default ARP entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefArpEntrySet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    GT_ETHERADDR                   arpMacAddr;

    /* convert MAC address to other format */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR));

    /* write a ARP MAC address to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpRouterArpAddrWrite");

    return rc;
};

/**
* @internal prvTgfDefTunnelStartEntryMplsForEgrQosSet function
* @endinternal
*
* @brief   Set a default X-over-MPLS tunnel start entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvTgfDefTunnelStartEntryMplsForEgrQosSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    /* set a Tunnel Start Entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.mplsCfg.tagEnable        = GT_TRUE;
    tunnelEntry.mplsCfg.vlanId           = PRV_TGF_NEXTHOPE_VLANID_CNS;
    cpssOsMemCpy(tunnelEntry.mplsCfg.macDa.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR));
    tunnelEntry.mplsCfg.ttl              = TS_TTL_FOR_LABELS_CNS;
    tunnelEntry.mplsCfg.exp1             = ORIG_OUTER_MPLS_EXP_CNS;
    tunnelEntry.mplsCfg.numLabels        = 1;
    tunnelEntry.mplsCfg.label1           = 0x66;
    tunnelEntry.mplsCfg.mplsEthertypeSelect = PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;
    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex, CPSS_TUNNEL_X_OVER_MPLS_E, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfTunnelStartEntrySet");
    return rc;
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelMplsPush function
* @endinternal
*
* @brief   MPLS LSR PUSH Functionality
*
* @param[in] pushType                 - Type of MPLS LSR PUSH Tunnel Start Test
*                                       None
*/
GT_VOID prvTgfTunnelMplsPush
(
    IN PRV_TGF_TUNNEL_MPLS_LSR_PUSH_TYPE_ENT pushType
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter;
    GT_U32    packetSize;
    GT_BOOL   isTtiSet = GT_FALSE;
    GT_U32    numTriggers = 0;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    PRV_UTF_LOG1_MAC("\nVLAN %d CONFIGURATION:\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1]);

    PRV_UTF_LOG1_MAC("VLAN %d CONFIGURATION:\n", PRV_TGF_NEXTHOPE_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("FDB CONFIGURATION:\n");
    PRV_UTF_LOG1_MAC("  MAC DA ...:34:02 to port [%d]\n", prvTgfPortsArray[1]);
    PRV_UTF_LOG1_MAC("  MAC DA ...:00:11 to port [%d]\n", prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("PCL CONFIGURATION:\n");
    PRV_UTF_LOG0_MAC("  CLEAR\n");

    PRV_UTF_LOG0_MAC("TTI CONFIGURATION:\n");
    PRV_UTF_LOG2_MAC("  FORWARD packet with MAC DA ..:34:02 to port [%d]\n"
            "  in VLAN %d with MIM tunnel (MAC DA must be ...:00:11)\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    PRV_UTF_LOG0_MAC("TRAFFIC GENERATION:\n");
    PRV_UTF_LOG0_MAC("  Sending MPLS packet with\n");
    switch (pushType) {
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E:
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_2_LABEL_E:
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_3_LABEL_E:
        PRV_UTF_LOG0_MAC("   label_1=0x66\n"); break;
    case PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E:
    case PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E:
        PRV_UTF_LOG0_MAC("   label_1=0x33\n   label_2=0x22\n   label_3=0x11\n"); break;
    default:
        break;
    }
    PRV_UTF_LOG1_MAC("   and MAC DA=...:34:02\n  to port [%d]\n",
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    PRV_UTF_LOG0_MAC("EXPECTED RESULTS:\n");
    PRV_UTF_LOG1_MAC("  packet is not forwarded in VLAN %d\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  packet is forwarded to port [%d] VLAN %d with ",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);
    switch (pushType) {
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E:
        PRV_UTF_LOG0_MAC("2 labels:\n"
            "   label_0=0x11(outer label)\n   lebel_1=0x66\n"); break;
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_2_LABEL_E:
        PRV_UTF_LOG0_MAC("3 labels:\n"
            "   label_0=0x22(outer label)\n   label_1=0x11\n   lebel_2=0x66\n"); break;
    case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_3_LABEL_E:
        PRV_UTF_LOG0_MAC("4 labels:\n"
            "   label_0=0x33(outer label)\n   label_1=0x22\n   label_2=0x11\n"
            "   lebel_3=0x66\n"); break;
    case PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E:
        PRV_UTF_LOG0_MAC("3 labels:\n"
            "   label_0=0x66(outer label)\n   label_1=0x22\n   lebel_2=0x11\n"); break;
    case PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E:
        PRV_UTF_LOG0_MAC("2 labels:\n"
            "   label_0=0x66(outer label)\n   lebel_2=0x11\n"); break;
    default:
        break;
    }
    PRV_UTF_LOG0_MAC("   and MAC DA=...:00:11\n\n");


    /* -------------------------------------------------------------------------
     * 1. Set configuration
     */

    /* set VLANs */
    rc = prvTgfDefBrigeConfSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefBrigeConfSet: %d", prvTgfDevNum);

    /* enable TTI lookup for MPLS key and MAC DA mode */
    if (GT_OK == rc) {
        rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             PRV_TGF_TTI_KEY_MPLS_E,
                             PRV_TGF_TTI_MAC_MODE_DA_E,
                             GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);
    }

    /* set TTI rules for MPLS Push key */
    if (GT_OK == rc) {
        rc = prvTgfDefTtiMplsPushRuleSet(pushType);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTtiMplsPushRuleSet: %d", prvTgfDevNum);
        isTtiSet = (GT_OK == rc);
    }

    /* set a X-over-MPLS Tunnel Start Entry */
    if (GT_OK == rc) {
        rc = prvTgfDefTunnelStartEntryMplsSet(pushType);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTunnelStartEntryMplsSet: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    if (GT_OK == rc)
    {
        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP counters */
            rc = prvTgfCountersIpReset();
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfCountersIpReset: %d", prvTgfDevNum);
        }

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* select MPLS packet to send */
        switch (pushType) {
        case PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E:
        case PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E:
            prvTgfPacketInfoArray[0] = &prvTgfPacketMpls3Info;
            break;
        default:
            prvTgfPacketInfoArray[0] = &prvTgfPacketMplsInfo;
            break;
        }

        /* enable capturing of receive port */
        rc = prvTgfPortCaptureEnable(
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

        /* generate traffic */
        rc = prvTgfTrafficGenerate();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrafficGenerate: %d", prvTgfDevNum);

        /* disable capturing of receive port */
        rc = prvTgfPortCaptureEnable(
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

        /* check ETH counters */
        for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
        {
            if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
            {
                /* packet after tunnel start in VLAN 6 */
                packetSize = prvTgfPacketInfoArray[0]->totalLen;
                switch (pushType) {
                case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E:
                    packetSize += TGF_MPLS_HEADER_SIZE_CNS;
                    break;
                case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_2_LABEL_E:
                    packetSize += 2 * TGF_MPLS_HEADER_SIZE_CNS;
                    break;
                case PRV_TGF_TUNNEL_MPLS_LSR_PUSH_3_LABEL_E:
                    packetSize += 3 * TGF_MPLS_HEADER_SIZE_CNS;
                    break;
                case PRV_TGF_TUNNEL_MPLS_LSR_SWAP_E:
                    break;
                case PRV_TGF_TUNNEL_MPLS_LSR_POP_SWAP_E:
                    packetSize -= TGF_MPLS_HEADER_SIZE_CNS;
                    break;
                default:
                    break;
                }
            }
            else
            {
                /* original packet before tunneling in VLAN 5 */
                packetSize = prvTgfPacketInfoArray[0]->totalLen;
            }

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(
                prvTgfDevNum,
                prvTgfPortsArray[portIter],
                prvTgfPacketsCountRxArray[0][portIter],
                prvTgfPacketsCountTxArray[0][portIter],
                packetSize, 1 /*burstCount*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* check IP counters */
            rc = prvTgfCountersIpCheck();
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfEthCountersCheck: %d", prvTgfDevNum);
        }

        /* print captured packets from receive port with prvTgfStartEntryMac */
        rc = prvTgfPortCapturePacketPrint(
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                prvTgfStartEntryMac, &numTriggers);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfPortCapturePacketPrint:"
                                     " port = %d, rc = 0x%02X\n",
                                     prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], rc);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
                prvTgfStartEntryMac[0], prvTgfStartEntryMac[1], prvTgfStartEntryMac[2],
                prvTgfStartEntryMac[3], prvTgfStartEntryMac[4], prvTgfStartEntryMac[5]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPortCapturePacketPrint: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    /* restore TTI lookup and MAC DA mode for Eth key */
    rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                         PRV_TGF_TTI_KEY_MPLS_E,
                         PRV_TGF_TTI_MAC_MODE_DA_E,
                         GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);

    /* restore common configuration */
    rc = prvTgfConfigurationRestore(GT_FALSE, isTtiSet);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfTunnelMplsPop function
* @endinternal
*
* @brief   MPLS LSR POP Functionality
*
* @param[in] popType                  - Type of MPLS LSR POP Tunnel Start Test
*                                       None
*/
GT_VOID prvTgfTunnelMplsPop
(
    IN PRV_TGF_TUNNEL_MPLS_LSR_POP_TYPE_ENT popType
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter;
    GT_U32    packetSize;
    GT_BOOL   isTtiSet = GT_FALSE;
    GT_U32    numTriggers = 0;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    PRV_UTF_LOG1_MAC("\nVLAN %d CONFIGURATION:\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1]);

    PRV_UTF_LOG1_MAC("VLAN %d CONFIGURATION:\n", PRV_TGF_NEXTHOPE_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("FDB CONFIGURATION:\n");
    PRV_UTF_LOG1_MAC("  MAC DA ...:34:02 to port [%d]\n", prvTgfPortsArray[1]);
    PRV_UTF_LOG1_MAC("  MAC DA ...:00:11 to port [%d]\n", prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("PCL CONFIGURATION:\n");
    PRV_UTF_LOG0_MAC("  CLEAR\n");

    PRV_UTF_LOG0_MAC("TTI CONFIGURATION:\n");
    PRV_UTF_LOG2_MAC("  FORWARD packet with MAC DA ..:34:02 to port [%d]\n"
            "  in VLAN %d with MIM tunnel (MAC DA must be ...:00:11)\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    PRV_UTF_LOG0_MAC("TRAFFIC GENERATION:\n");
    PRV_UTF_LOG0_MAC("  Sending MPLS packet with 3 labels:\n"
                     "   label_0=0x33(outer label)\n   label_1=0x22\n   label_2=0x11\n");
    PRV_UTF_LOG1_MAC("   and MAC DA=...:34:02\n  to port [%d]\n",
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    PRV_UTF_LOG0_MAC("EXPECTED RESULTS:\n");
    PRV_UTF_LOG1_MAC("  packet is not forwarded in VLAN %d\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  packet is forwarded to port [%d] VLAN %d with ",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);
    if(PRV_TGF_TUNNEL_MPLS_LSR_POP_1_LABEL_E == popType)
        PRV_UTF_LOG0_MAC("2 labels:\n   label_0=0x22(outer label)\n   lebel_1=0x11\n");
    else if(PRV_TGF_TUNNEL_MPLS_LSR_POP_2_LABEL_E == popType)
        PRV_UTF_LOG0_MAC("1 labels:\n   label_0=0x11(outer label)\n");
    PRV_UTF_LOG0_MAC("   and MAC DA=...:00:11\n\n");


    /* -------------------------------------------------------------------------
     * 1. Set configuration
     */

    /* set VLANs */
    rc = prvTgfDefBrigeConfSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefBrigeConfSet: %d", prvTgfDevNum);

    /* enable TTI lookup for MPLS key and MAC DA mode */
    if (GT_OK == rc) {
        rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             PRV_TGF_TTI_KEY_MPLS_E,
                             PRV_TGF_TTI_MAC_MODE_DA_E,
                             GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);
    }

    /* set TTI rules for MPLS Pop key */
    if (GT_OK == rc) {
        rc = prvTgfDefTtiMplsPopRuleSet(popType);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTtiMplsPopRuleSet: %d", prvTgfDevNum);
        isTtiSet = (GT_OK == rc);
    }

    /* set ARP Entry (Write ARP address 00:00:00:00:00:11 at index 1) */
    if (GT_OK == rc) {
        rc = prvTgfDefArpEntrySet();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefArpEntrySet: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    if (GT_OK == rc)
    {
        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP counters */
            rc = prvTgfCountersIpReset();
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfCountersIpReset: %d", prvTgfDevNum);
        }

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* select MPLS packet to send */
        prvTgfPacketInfoArray[0] = &prvTgfPacketMpls3Info;

        /* enable capturing of receive port */
        rc = prvTgfPortCaptureEnable(
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

        /* generate traffic */
        rc = prvTgfTrafficGenerate();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrafficGenerate: %d", prvTgfDevNum);

        /* disable capturing of receive port */
        rc = prvTgfPortCaptureEnable(
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

        /* check ETH counters */
        for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
        {
            if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
            {
                /* packet after tunnel start in VLAN 6 */
                packetSize = prvTgfPacketInfoArray[0]->totalLen;
                switch (popType) {
                case PRV_TGF_TUNNEL_MPLS_LSR_POP_1_LABEL_E:
                    packetSize -= TGF_MPLS_HEADER_SIZE_CNS;
                    break;
                case PRV_TGF_TUNNEL_MPLS_LSR_POP_2_LABEL_E:
                    packetSize -= 2 * TGF_MPLS_HEADER_SIZE_CNS;
                    break;
                default:
                    break;
                }
            }
            else
            {
                /* original packet before tunneling in VLAN 5 */
                packetSize = prvTgfPacketInfoArray[0]->totalLen;
            }

            if (PRV_TGF_SEND_PORT_IDX_CNS != portIter)
            {
                packetSize -= TGF_VLAN_TAG_SIZE_CNS;
            }

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(
                prvTgfDevNum,
                prvTgfPortsArray[portIter],
                prvTgfPacketsCountRxArray[0][portIter],
                prvTgfPacketsCountTxArray[0][portIter],
                packetSize, 1 /*burstCount*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* check IP counters */
            rc = prvTgfCountersIpCheck();
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfEthCountersCheck: %d", prvTgfDevNum);
        }

        /* print captured packets from receive port with prvTgfStartEntryMac */
        rc = prvTgfPortCapturePacketPrint(
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                prvTgfStartEntryMac, &numTriggers);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfPortCapturePacketPrint:"
                                     " port = %d, rc = 0x%02X\n",
                                     prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], rc);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
                prvTgfStartEntryMac[0], prvTgfStartEntryMac[1], prvTgfStartEntryMac[2],
                prvTgfStartEntryMac[3], prvTgfStartEntryMac[4], prvTgfStartEntryMac[5]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPortCapturePacketPrint: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    /* restore TTI lookup and MAC DA mode for Eth key */
    rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                         PRV_TGF_TTI_KEY_MPLS_E,
                         PRV_TGF_TTI_MAC_MODE_DA_E,
                         GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);

    /* restore common configuration */
    rc = prvTgfConfigurationRestore(GT_FALSE, isTtiSet);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal prvTgfTunnelMplsPushAndEgrQosCfgAdd function
* @endinternal
*
* @brief   MPLS LSR PUSH and adding egress qos configuration
*/
GT_VOID prvTgfTunnelMplsPushAndEgrQosCfgAdd
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portIter;
    GT_U32    packetSize;
    GT_BOOL   isTtiSet = GT_FALSE;


    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];
    GT_U32                  numTriggers = 0;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    PRV_UTF_LOG1_MAC("\nVLAN %d CONFIGURATION:\n", PRV_TGF_SEND_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1]);

    PRV_UTF_LOG1_MAC("VLAN %d CONFIGURATION:\n", PRV_TGF_NEXTHOPE_VLANID_CNS);
    PRV_UTF_LOG2_MAC("  Port members: [%d], [%d]\n",
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("FDB CONFIGURATION:\n");
    PRV_UTF_LOG1_MAC("  MAC DA ...:34:02 to port [%d]\n", prvTgfPortsArray[1]);
    PRV_UTF_LOG1_MAC("  MAC DA ...:00:11 to port [%d]\n", prvTgfPortsArray[3]);

    PRV_UTF_LOG0_MAC("PCL CONFIGURATION:\n");
    PRV_UTF_LOG0_MAC("  CLEAR\n");

    PRV_UTF_LOG0_MAC("TTI CONFIGURATION:\n");
    PRV_UTF_LOG2_MAC("  FORWARD packet with MAC DA ..:34:02 to port [%d]\n"
            "  in VLAN %d with MIM tunnel (MAC DA must be ...:00:11)\n",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);

    PRV_UTF_LOG0_MAC("TRAFFIC GENERATION:\n");
    PRV_UTF_LOG0_MAC("  Sending MPLS packet with\n");

    PRV_UTF_LOG0_MAC("   label_1=0x66\n");

    PRV_UTF_LOG1_MAC("   and MAC DA=...:34:02\n  to port [%d]\n",
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    PRV_UTF_LOG0_MAC("EXPECTED RESULTS:\n");
    PRV_UTF_LOG2_MAC("  packet is forwarded to port [%d] VLAN %d with ",
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_NEXTHOPE_VLANID_CNS);
    PRV_UTF_LOG0_MAC("   MAC DA=...:00:11\n\n");


    /* -------------------------------------------------------------------------
     * 1. Set configuration
     */

    /* set VLANs */
    rc = prvTgfDefBrigeConfSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefBrigeConfSet: %d", prvTgfDevNum);

    /* enable TTI lookup for MPLS key and MAC DA mode */
    if (GT_OK == rc) {
        rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             PRV_TGF_TTI_KEY_MPLS_E,
                             PRV_TGF_TTI_MAC_MODE_DA_E,
                             GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);
    }

    /* set TTI rules for MPLS Push key */
    if (GT_OK == rc) {
        rc = prvTgfDefTtiMplsPushRuleSet(PRV_TGF_TUNNEL_MPLS_LSR_PUSH_1_LABEL_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTtiMplsPushRuleSet: %d", prvTgfDevNum);
        isTtiSet = (GT_OK == rc);
    }

    /* set a X-over-MPLS Tunnel Start Entry */
    if (GT_OK == rc) {
        rc = prvTgfDefTunnelStartEntryMplsForEgrQosSet();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefTunnelStartEntryMplsSet: %d", prvTgfDevNum);
    }
    /* add exp egress mapping configuration */
    prvTgfTunnelMplsQosSet(prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],GT_TRUE/*set remap*/);

    /* -------------------------------------------------------------------------
     * 2. Generate traffic and Check counters
     */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* MPLS packet to send */
    prvTgfPacketInfoArray[0] = &prvTgfPacketMplsInfo;

    /* enable capturing of receive port */
    rc = prvTgfPortCaptureEnable(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

    /* generate traffic */
    rc = prvTgfTrafficGenerate();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrafficGenerate: %d", prvTgfDevNum);

    /* disable capturing of receive port */
    rc = prvTgfPortCaptureEnable(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

    /* check ETH counters */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            /* packet after tunnel start in VLAN 6 */
            packetSize = prvTgfPacketInfoArray[0]->totalLen +
            TGF_MPLS_HEADER_SIZE_CNS;
        }
        else
        {
            /* original packet before tunneling in VLAN 5 */
            packetSize = prvTgfPacketInfoArray[0]->totalLen;
        }

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            prvTgfPacketsCountRxArray[0][portIter],
            prvTgfPacketsCountTxArray[0][portIter],
            packetSize, 1 /*burstCount*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* get Trigger Counters */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* set vfd for destination MAC */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS +
                         TGF_ETHERTYPE_SIZE_CNS;

    /* the expected outer label is : 00 06 64 ee */
    {
        GT_U32 ttl,exp1,label1,sBit;
        GT_U32 mplsLabel;
        ttl    = TS_TTL_FOR_LABELS_CNS;
        exp1   = OUTER_MPLS_EXP_AFTER_EXP_REMAP_CNS; /*mapped from ORIG_OUTER_MPLS_EXP_CNS*/
        label1 = 0x66;
        sBit = 0;

        PRV_UTF_LOG1_MAC("expected outer ttl[0x%x] \n" , ttl);
        PRV_UTF_LOG1_MAC("expected outer exp[%d] \n" , exp1);
        PRV_UTF_LOG1_MAC("expected outer label[0x%x] \n" , label1);
        PRV_UTF_LOG1_MAC("expected outer sBit[%d] \n" , sBit);

        mplsLabel = ttl | (sBit << 8) | (exp1 << 9) | (label1 << 12);
        PRV_UTF_LOG1_MAC("expected outer label[0x%8.8x]",mplsLabel);

        vfdArray[0].cycleCount = 4;
        vfdArray[0].patternPtr[0] = (GT_U8)(mplsLabel >> 24);
        vfdArray[0].patternPtr[1] = (GT_U8)(mplsLabel >> 16);
        vfdArray[0].patternPtr[2] = (GT_U8)(mplsLabel >>  8);
        vfdArray[0].patternPtr[3] = (GT_U8)(mplsLabel >>  0);
    }
    /* get trigger counters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers,
                     "get another control word that expected");


    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    /* restore TTI lookup and MAC DA mode for Eth key */
    rc = prvTgfTtiEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                         PRV_TGF_TTI_KEY_MPLS_E,
                         PRV_TGF_TTI_MAC_MODE_DA_E,
                         GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiEnable: %d", prvTgfDevNum);

    /* restore common configuration */
    rc = prvTgfConfigurationRestore(GT_FALSE, isTtiSet);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(
    GT_OK, rc, "prvTgfTunnelStartMplsPwControlWordSet: %d", prvTgfDevNum);
    /* egress qos config restore */
    prvTgfTunnelMplsQosSet(prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],GT_FALSE/*restore no remap*/);
}

/**
* @internal prvTgfTunnelMplsQosSet function
* @endinternal
*
* @brief   MPLS Egress QoS mapping set
*
* @param[in] egressPort               - the port to set configuration on.
* @param[in] doRemap                  - do we set/restore the exp-to-exp remap
*                                       None
*/
GT_VOID prvTgfTunnelMplsQosSet
(
    IN GT_U32  egressPort,
    IN GT_BOOL doRemap
)
{
    GT_STATUS rc;
    GT_U32  mappingTableIndex = 5;
    GT_U32  inExp = ORIG_OUTER_MPLS_EXP_CNS , outExp = OUTER_MPLS_EXP_AFTER_EXP_REMAP_CNS;

    if(doRemap == GT_TRUE)
    {
        /* get egress QoS EXP2EXP mapping enable state */
        rc = prvTgfCosPortEgressQoSExpMappingEnableGet(prvTgfDevNum, egressPort, &egressQosExpEnableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                   "prvTgfCosPortEgressQoSExpMappingEnableGet: %d, %d", prvTgfDevNum, egressPort);
        /* set egress QoS EXP2EXP mapping enable state */
        rc = prvTgfCosPortEgressQoSExpMappingEnableSet(prvTgfDevNum, egressPort, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSExpMappingEnableSet: %d, %d", prvTgfDevNum, egressPort);
        /* get egress eport  mapping table select */
        rc = prvTgfCosPortEgressQoSMappingTableIndexGet(prvTgfDevNum, egressPort,&egressMappingTableIndexGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSMappingTableIndexGet: %d, %d", prvTgfDevNum, egressPort);
        /* set egress eport  mapping table select */
        rc = prvTgfCosPortEgressQoSMappingTableIndexSet(prvTgfDevNum, egressPort,mappingTableIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSMappingTableIndexSet: %d, %d", prvTgfDevNum, egressPort);



        /* get egress eport exp2exp mapping */
         rc = prvTgfCosEgressExp2ExpMappingEntryGet(prvTgfDevNum, mappingTableIndex, inExp, &exp2expGet);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressExp2ExpMappingEntryGet : %d",prvTgfDevNum);
         /* set egress eport exp2exp mapping */
         rc = prvTgfCosEgressExp2ExpMappingEntrySet(prvTgfDevNum, mappingTableIndex, inExp, outExp);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressExp2ExpMappingEntrySet : %d",prvTgfDevNum);
    }
    else
    {
        /* set egress QoS EXP2EXP mapping enable state */
        rc = prvTgfCosPortEgressQoSExpMappingEnableSet(prvTgfDevNum, egressPort, egressQosExpEnableGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSExpMappingEnableSet: %d, %d", prvTgfDevNum, egressPort);
        /* set egress eport  mapping table select */
        rc = prvTgfCosPortEgressQoSMappingTableIndexSet(prvTgfDevNum, egressPort,egressMappingTableIndexGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfCosPortEgressQoSMappingTableIndexSet: %d, %d", prvTgfDevNum, egressPort);
        /* set egress eport exp2exp mapping */
        rc = prvTgfCosEgressExp2ExpMappingEntrySet(prvTgfDevNum, mappingTableIndex, inExp, exp2expGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfCosEgressExp2ExpMappingEntrySet : %d",prvTgfDevNum);
    }

}


/**
* @internal tsMplsEgressQosMappingPassengerEthernet_configTs function
* @endinternal
*
* @brief   config :MPLS TS
*         for Ethernet passenger + egress EXP-to-EXP remap by egress EPort
* @param[in] isFirstTime              - indication to 'save' previous settings for the 'restore'
* @param[in] isTsVlanTagged           - is the TS need vlan tag
*                                       None
*/
static GT_VOID tsMplsEgressQosMappingPassengerEthernet_configTs
(
    IN GT_BOOL isFirstTime,
    IN GT_BOOL isTsVlanTagged
)
{
    GT_STATUS                      rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
    GT_ETHERADDR macSaAddr;

    /* set a Tunnel Start Entry
     * cpssDxChTunnelStartEntrySet(,,CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC)
     */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.mplsCfg.tagEnable        = isTsVlanTagged;
    tunnelEntry.mplsCfg.vlanId           = PRV_TGF_MPLS_TS_VLANID_CNS;
    cpssOsMemCpy(tunnelEntry.mplsCfg.macDa.arEther,
                 prvTgfPacketTunnelStartMplsL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));

    tunnelEntry.mplsCfg.numLabels    = 3;

    tunnelEntry.mplsCfg.ttl          = TS_TTL_FOR_LABELS_CNS;
    tunnelEntry.mplsCfg.label1       = prvTgfPacketMplsMpls1Part.label;
    tunnelEntry.mplsCfg.exp1         = prvTgfPacketMplsMpls1Part.exp;

    tunnelEntry.mplsCfg.label2       = prvTgfPacketMplsMpls2Part.label;
    tunnelEntry.mplsCfg.exp2         = prvTgfPacketMplsMpls2Part.exp;

    tunnelEntry.mplsCfg.label3       = prvTgfPacketMplsMpls3Part.label;
    tunnelEntry.mplsCfg.exp3         = prvTgfPacketMplsMpls3Part.exp;

    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex, CPSS_TUNNEL_X_OVER_MPLS_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet");

    if(isFirstTime == GT_TRUE)
    {
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,egressPort,&origEgressInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoGet");
    }

    /* set the egress eport to use TS for the ethernet passenger */
    egressInfo = origEgressInfo;
    egressInfo.tunnelStart = GT_TRUE;
    egressInfo.tunnelStartPtr = prvTgfTunnelStartLineIndex;
    egressInfo.tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,egressPort,&egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    if(isFirstTime == GT_TRUE)
    {
        rc = prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum,egressPort,&origRouterMacSaIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");
    }

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,egressPort,testRouterMacSaIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    if(isFirstTime == GT_TRUE)
    {
        rc = prvTgfIpRouterGlobalMacSaGet(prvTgfDevNum,testRouterMacSaIndex,&origMacSaAddr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");
    }

    /* the mac SA for the TS header */
    macSaAddr.arEther[0] =  prvTgfPacketTunnelStartMplsL2Part.saMac[0];
    macSaAddr.arEther[1] =  prvTgfPacketTunnelStartMplsL2Part.saMac[1];
    macSaAddr.arEther[2] =  prvTgfPacketTunnelStartMplsL2Part.saMac[2];
    macSaAddr.arEther[3] =  prvTgfPacketTunnelStartMplsL2Part.saMac[3];
    macSaAddr.arEther[4] =  prvTgfPacketTunnelStartMplsL2Part.saMac[4];
    macSaAddr.arEther[5] =  prvTgfPacketTunnelStartMplsL2Part.saMac[5];

    rc = prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,testRouterMacSaIndex,&macSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");

    return ;
};

/**
* @internal tsMplsPassengerEthernet_config function
* @endinternal
*
* @brief   configuration function for :
*         MPLS TS for Ethernet passenger configuration
*/
static GT_VOID tsMplsPassengerEthernet_config
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  ingressPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    /* -------------------------------------------------------------------------
     * 1. Set configuration
     */

    /* set VLANs */
    rc = prvTgfDefBrigeConfSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfDefBrigeConfSet: %d", prvTgfDevNum);
    /* set the egress port as outer tag 1 inner tag 0 */
    rc = prvTgfBrgVlanTagMemberAdd(PRV_TGF_SEND_VLANID_CNS,
        egressPort,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagMemberAdd: %d", prvTgfDevNum);

    /* set other ports also in vlan */
    rc = prvTgfBrgVlanTagMemberAdd(PRV_TGF_SEND_VLANID_CNS,
        prvTgfPortsArray[PRV_TGF_FREE_PORT_IDX_CNS],
        PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagMemberAdd: %d", prvTgfDevNum);

    /* set other ports also in vlan */
    rc = prvTgfBrgVlanTagMemberAdd(PRV_TGF_SEND_VLANID_CNS,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagMemberAdd: %d", prvTgfDevNum);


    /* save for restoring the default vid1 for the ingress port */
    prvTgfBrgVlanPortVid1Get(prvTgfDevNum,
        ingressPort,
        CPSS_DIRECTION_INGRESS_E,
        &origVid1);

    /* set default vid1 for the ingress port */
    prvTgfBrgVlanPortVid1Set(prvTgfDevNum,
        ingressPort,
        CPSS_DIRECTION_INGRESS_E,
        PRV_TGF_VLANID1_CNS);

    origTtl1 = prvTgfPacketMplsMpls1Part.timeToLive;
    origTtl2 = prvTgfPacketMplsMpls2Part.timeToLive;
    origTtl3 = prvTgfPacketMplsMpls3Part.timeToLive;

    prvTgfPacketMplsMpls1Part.timeToLive = TS_TTL_FOR_LABELS_CNS;
    prvTgfPacketMplsMpls2Part.timeToLive = TS_TTL_FOR_LABELS_CNS;
    prvTgfPacketMplsMpls3Part.timeToLive = TS_TTL_FOR_LABELS_CNS;
}

/**
* @internal tsMplsEgressQosMappingPassengerEthernet_config function
* @endinternal
*
* @brief   configuration function for :
*         MPLS TS for Ethernet passenger + egress EXP-to-EXP remap by egress EPort
*         configuration
*/
static GT_VOID tsMplsEgressQosMappingPassengerEthernet_config
(
    GT_VOID
)
{
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    tsMplsPassengerEthernet_config();
    /* set egress remark EXP to EXP settings */
    prvTgfTunnelMplsQosSet(egressPort,GT_TRUE);

    /* set TS entry of MPLS with vlan tag and 3 labels */
    tsMplsEgressQosMappingPassengerEthernet_configTs(GT_TRUE,GT_TRUE/*TS with vlan tag*/);

}
/**
* @internal tsMplsEgressQosMappingPassengerEthernet_restore function
* @endinternal
*
* @brief   restore function , for :
*         MPLS TS for Ethernet passenger + egress EXP-to-EXP remap by egress EPort
*/
static GT_VOID tsMplsEgressQosMappingPassengerEthernet_restore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    /* restore default vid1 for the ingress port */
    prvTgfBrgVlanPortVid1Set(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_DIRECTION_INGRESS_E,
        origVid1);

    /* restore common configuration */
    rc = prvTgfConfigurationRestore(GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

    /* restore value */
    prvTgfPacketMplsMpls3Part.exp = ORIG_OUTER_MPLS_EXP_CNS;

    prvTgfPacketMplsMpls1Part.timeToLive = origTtl1;
    prvTgfPacketMplsMpls2Part.timeToLive = origTtl2;
    prvTgfPacketMplsMpls3Part.timeToLive = origTtl3;

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,egressPort,&origEgressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,egressPort,origRouterMacSaIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,testRouterMacSaIndex,&origMacSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");

    /* reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

}

/**
* @internal tsMplsEgressQosMappingPassengerEthernet_send function
* @endinternal
*
* @brief   send packet function , for :
*         MPLS TS for Ethernet passenger + egress EXP-to-EXP remap by egress EPort
*/
static GT_VOID tsMplsEgressQosMappingPassengerEthernet_send
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32  ingressPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    /* reset counters :
    to clean the CPU captured table and to set 'link up' to the needed ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* enable capturing of receive port */
    rc = prvTgfPortCaptureEnable(egressPort, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketEthernetInfo,
        1 /*burstCount*/, 0 /*numVfd*/, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, ingressPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

    /* disable capturing of receive port */
    rc = prvTgfPortCaptureEnable(egressPort, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);
}

/**
* @internal tsMplsEgressQosMappingPassengerEthernet_checkResult function
* @endinternal
*
* @brief   check results of egress packet (EXP remap) function , for :
*         MPLS TS for Ethernet passenger + egress EXP-to-EXP remap by egress EPort
* @param[in] didRemap                 - do we expect the EXP to do remap
*                                       None
*/
static GT_VOID tsMplsEgressQosMappingPassengerEthernet_checkResult
(
    IN GT_BOOL didRemap
)
{
    GT_STATUS  rc;
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets = 0,numOfExpectedPackets = 1;


    /* portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = egressPort;

    /* next modify the 'Expected packet' :
        prvTgfPacketExpectedTsMplsPassengerEthernetInfo */
    if(didRemap == GT_TRUE)
    {
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
            prvTgfPacketMplsMpls3Part.exp = OUTER_MPLS_EXP_AFTER_EXP_REMAP_CNS;
        }
        else
        {
            /* Errata : egress packet is MPLS but passenger is not:
               so use basic EXP value                            */
            prvTgfPacketMplsMpls3Part.exp = BASIC_EXP_CNS;
        }
    }
    else
    {
        prvTgfPacketMplsMpls3Part.exp = ORIG_OUTER_MPLS_EXP_CNS;
    }

    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(&portInterface,
            &prvTgfPacketExpectedTsMplsPassengerEthernetInfo,
            numOfExpectedPackets,/*numOfPackets*/
            0,/*numVfd*/
            NULL,/*vfdArray[]*/
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
}


/**
* @internal prvTgfTunnelStartMplsEgressQosMappingPassengerEthernet function
* @endinternal
*
* @brief   MPLS TS for Ethernet passenger + egress EXP-to-EXP remap by egress EPort
*         configuration
*         config + send traffic + verify results + restore
*/
GT_VOID prvTgfTunnelStartMplsEgressQosMappingPassengerEthernet
(
    GT_VOID
)
{
    /* set egress remark EXP to EXP settings */
    tsMplsEgressQosMappingPassengerEthernet_config();

    /* send and check the Exp-to-Exp done */
    tsMplsEgressQosMappingPassengerEthernet_send();
    tsMplsEgressQosMappingPassengerEthernet_checkResult(GT_TRUE);

    /* remove egress remark EXP to EXP settings */
    prvTgfTunnelMplsQosSet(prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],GT_FALSE);

    /* send and check that NO Exp-to-Exp done */
    tsMplsEgressQosMappingPassengerEthernet_send();
    tsMplsEgressQosMappingPassengerEthernet_checkResult(GT_FALSE);

    /* restore full configurations*/
    tsMplsEgressQosMappingPassengerEthernet_restore();

}

/**
* @internal tsMplsMplsModifyOuterVlanTagPassengerEthernet_config_epcl function
* @endinternal
*
* @brief   Adds Egress Pcl configuration
*/
static GT_VOID tsMplsMplsModifyOuterVlanTagPassengerEthernet_config_epcl
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_TGF_EPCL_PCL_ID_CNS;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: init PCL Engine for receive port 2: */
    /* AUTODOC:   egress direction, lookup0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key EGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInitExt2(
        egressPort,
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
        egressPort);

    /* Egress rule */
    /* Forward, modify VID0  */
    /* match fields          */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    ruleIndex  = PRV_TGF_EPCL_RULE_INDEX_CNS;

    /* (simple) match only according to the PCL-ID */
    mask.ruleEgrStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleEgrStdNotIp.common.pclId = PRV_TGF_EPCL_PCL_ID_CNS;

    /*(simple) action to modify the vid,up on the outer tag (the one on the TS and not in the passenger) */
    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;

    /* action to modify the vid,up on the outer tag (the one on the TS and not in the passenger) */
    action.vlan.egressVlanId0Cmd =
        PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
    action.vlan.vlanId  = PRV_TGF_EPCL_OUTER_TAG_MODIFIED_VID_CNS;
    action.qos.egressUp0Cmd = PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
    action.qos.up       = PRV_TGF_EPCL_OUTER_TAG_MODIFIED_UP_CNS;

    /* AUTODOC: set PCL rule 3 with: */
    /* AUTODOC:   format EGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   pattern pclId=2, TAG1 UP=3, CFI=1, VID=10 */
    /* AUTODOC:   enable modify OUTER_TAG, VID=4 */
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex,
        0,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
        prvTgfDevNum, ruleFormat, ruleIndex);


   /* AUTODOC : enables egress policy for TS packets on the egress port */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum,egressPort,
        PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPclEgressPclPacketTypesSet");

   /* AUTODOC : allow TS packets to access the EPCL configuration table with 'port mode' (not forced to 'vlan mode') */
    prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");



}

/**
* @internal tsMplsMplsModifyOuterVlanTagPassengerEthernet_config function
* @endinternal
*
* @brief   configuration function for :
*         MPLS TS for Ethernet passenger + EPCL for 'modify outer tag' up,vid
* @param[in] isTsVlanTagged           - is the TS need vlan tag
*                                       None
*/
static GT_VOID tsMplsMplsModifyOuterVlanTagPassengerEthernet_config
(
    IN GT_BOOL skipInit,
    IN GT_BOOL isTsVlanTagged
)
{

    if(skipInit == GT_FALSE)
    {
        tsMplsPassengerEthernet_config();
        /* set the EPCL init , rule and action */
        tsMplsMplsModifyOuterVlanTagPassengerEthernet_config_epcl();
    }

    /* set TS entry of MPLS 3 labels and with/without vlan tag */
    tsMplsEgressQosMappingPassengerEthernet_configTs(
        ((skipInit == GT_FALSE) ? GT_TRUE : GT_FALSE),
        isTsVlanTagged);
}

/**
* @internal tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_send function
* @endinternal
*
* @brief   send ethernet packet .(enable capture to CPU on egress port)
*/
static GT_VOID tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_send
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32  ingressPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    /* reset counters :
    to clean the CPU captured table and to set 'link up' to the needed ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* enable capturing of receive port */
    rc = prvTgfPortCaptureEnable(egressPort, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketEthernetInfo,
        1 /*burstCount*/, 0 /*numVfd*/, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, ingressPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

    /* disable capturing of receive port */
    rc = prvTgfPortCaptureEnable(egressPort, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPortCaptureEnable: %d", prvTgfDevNum);
}

/**
* @internal tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_checkResult function
* @endinternal
*
* @brief   check results of egress packet (outer vlan tag modification) function , for :
*         MPLS TS for Ethernet passenger + EPCL 'modify outer vlan tag'
* @param[in] isTsVlanTagged           - is the TS need vlan tag
*                                       None
*/
static GT_VOID tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_checkResult
(
    IN GT_BOOL isTsVlanTagged
)
{
    GT_STATUS  rc;
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets = 0,numOfExpectedPackets = 1;
    TGF_PRI             orig_pri = prvTgfPacketTunnelStartMplsVlanTagPart.pri;
    TGF_VLAN_ID         orig_vid = prvTgfPacketTunnelStartMplsVlanTagPart.vid;
    GT_U32      tsVidPartIndex = 1;

    /* portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = egressPort;

    /* next modify the 'Expected packet' :
        {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelStartMplsVlanTagPart},
        prvTgfPacketExpectedTsMplsPassengerEthernetInfo */
    if(isTsVlanTagged == GT_TRUE)
    {
        /* the TS is with vlan tag */
        prvTgfPacketExpectedTsMplsPassengerEthernetPartArray[tsVidPartIndex].type = TGF_PACKET_PART_VLAN_TAG_E;
        prvTgfPacketTunnelStartMplsVlanTagPart.pri = PRV_TGF_EPCL_OUTER_TAG_MODIFIED_UP_CNS;
        prvTgfPacketTunnelStartMplsVlanTagPart.vid = PRV_TGF_EPCL_OUTER_TAG_MODIFIED_VID_CNS;
    }
    else
    {
        /* the TS is without vlan tag */
        prvTgfPacketExpectedTsMplsPassengerEthernetPartArray[tsVidPartIndex].type = TGF_PACKET_PART_SKIP_E;
    }

    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(&portInterface,
            &prvTgfPacketExpectedTsMplsPassengerEthernetInfo,
            numOfExpectedPackets,/*numOfPackets*/
            0,/*numVfd*/
            NULL,/*vfdArray[]*/
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /*restore values*/
    prvTgfPacketExpectedTsMplsPassengerEthernetPartArray[tsVidPartIndex].type = TGF_PACKET_PART_VLAN_TAG_E;
    prvTgfPacketTunnelStartMplsVlanTagPart.pri = orig_pri ;
    prvTgfPacketTunnelStartMplsVlanTagPart.vid = orig_vid ;
}

/**
* @internal tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_restore_epcl function
* @endinternal
*
* @brief   Restore epcl test configuration
*/
static GT_VOID tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_restore_epcl
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC : invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
        CPSS_PCL_RULE_SIZE_STD_E,
        PRV_TGF_EPCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
        prvTgfDevNum, GT_TRUE);

   /* AUTODOC : restore TS packets to access the EPCL configuration table with 'vlan mode' */
    prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");

   /* AUTODOC : disable all PCL configurations on the ports */
   prvTgfPclPortsRestoreAll();
}

/**
* @internal tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_restore function
* @endinternal
*
* @brief   restore function , for :
*         MPLS TS for Ethernet passenger + EPCL modify outer vlan tag
*/
static GT_VOID tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_restore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  egressPort = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    /* restore default vid1 for the ingress port */
    prvTgfBrgVlanPortVid1Set(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_DIRECTION_INGRESS_E,
        origVid1);

    /* restore common configuration */
    rc = prvTgfConfigurationRestore(GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);

    /* restore value */
    prvTgfPacketMplsMpls1Part.timeToLive = origTtl1;
    prvTgfPacketMplsMpls2Part.timeToLive = origTtl2;
    prvTgfPacketMplsMpls3Part.timeToLive = origTtl3;

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,egressPort,&origEgressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum,egressPort,origRouterMacSaIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum,testRouterMacSaIndex,&origMacSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");

    /* reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);


    /* restore EPCL */
    tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_restore_epcl();



}

/**
* @internal prvTgfTunnelStartMplsModifyOuterVlanTagPassengerEthernet function
* @endinternal
*
* @brief   Test that checking 'outer vlan tag' modifications on TS packets.
*         config + send traffic + verify results + restore
*/
GT_VOID prvTgfTunnelStartMplsModifyOuterVlanTagPassengerEthernet
(
    GT_VOID
)
{
    GT_BOOL isTsVlanTagged;/*is the TS need vlan tag*/

    isTsVlanTagged = GT_TRUE;
    /* set the init for bridging,EPCL to the egress port */
    /* set the TS to have vlan tag */
    tsMplsMplsModifyOuterVlanTagPassengerEthernet_config(GT_FALSE , isTsVlanTagged);

    /* send and check the vid,up are modified on the TS */
    tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_send();
    tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_checkResult(isTsVlanTagged);

    isTsVlanTagged = GT_FALSE;
    /* set the TS to have NO vlan tag */
    tsMplsMplsModifyOuterVlanTagPassengerEthernet_config(GT_TRUE , isTsVlanTagged);

    /* send and check the no modifications in the packet */
    tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_send();
    tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_checkResult(isTsVlanTagged);

    /* restore full configurations*/
    tsMplsEgressMplsModifyOuterVlanTagPassengerEthernet_restore();
}




