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
* @file prvTgfIpNonExactMatchRoutingIpv4Mc.c
*
* @brief Non-exact match IPv4 MC prefix routing.
*
* @version   10
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
#include <ip/prvTgfIpNonExactMatchRoutingIpv4Mc.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Send VLAN */
#define PRV_TGF_SEND_VLANID_CNS           5

/* Nexthop VLAN */
#define PRV_TGF_NEXTHOP_MC_VLANID_CNS     6

/* Default VLAN */
#define PRV_TGF_DEFAULT_VLANID_CNS        7

/* Port index to send traffic */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* Port indexes to receive MC traffic */
#define PRV_TGF_MC_RCV1_PORT_IDX_CNS      1
#define PRV_TGF_MC_RCV2_PORT_IDX_CNS      2

/* Port index to receive default traffic */
#define PRV_TGF_DEFAULT_PORT_IDX_CNS      3

/* Default vidx number */
#define PRV_TGF_DEF_VIDX_CNS              0

#ifdef GM_USED
/* Number of packets to send */
#define PRV_TGF_BURST_COUNT_CNS          4

/* Max src prefix length */
#define PRV_TGF_MAX_SRC_PREFIX_LEN_CNS   5

/* Max grp prefix length */
#define PRV_TGF_MAX_GRP_PREFIX_LEN_CNS   32

/* Number of byte iteration to send */
#define PRV_TGF_BYTE_ITER__CNS           1

/* Number of byte iteration to send */
#define PRV_TGF_BIT_ITER__CNS            4
#else
/* Number of packets to send */
#define PRV_TGF_BURST_COUNT_CNS          32

/* Max src prefix length */
#define PRV_TGF_MAX_SRC_PREFIX_LEN_CNS   32

/* Max grp prefix length */
#define PRV_TGF_MAX_GRP_PREFIX_LEN_CNS   32

/* Number of byte iteration to send */
#define PRV_TGF_BYTE_ITER__CNS           4

/* Number of byte iteration to send */
#define PRV_TGF_BIT_ITER__CNS            8
#endif

/* Default MLL pair pointer index */
static GT_U32 prvTgfDefMllPointerIndex  = 0;

/* MLL pair pointer index */
static GT_U32 prvTgfMllPointerIndex     = 2;

/* Default Route entry index for MC Route entry Table */
static GT_U32 prvTgfDefRouteEntryIndex  = 1;

/* The Route entry index for MC Route entry Table */
static GT_U32 prvTgfRouteEntryIndex     = 10;

/* the LPM DB id for LPM Table */
static GT_U32 prvTgfLpmDBId             = 0;

/* the Virtual Router id for LPM Table */
static GT_U32 prvTgfVrId                = 0;

/* Base multicast group IP address */
static TGF_IPV4_ADDR prvTgfBaseMcGrpIpv4 = {224, 1, 1, 1};

/* Base multicast source IP address */
static TGF_IPV4_ADDR prvTgfBaseMcSrcIpv4 = {10, 153, 102, 170};

/****************************** Test packets *********************************/

/* L2 part of IPv4 MC packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartMcIpv4 =
{
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},  /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}   /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,      /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS         /* pri, cfi, VlanId */
};

/* Packet's IPv4 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartIpv4 =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* Packet's MC IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketMcIpv4Part =
{
    4,                                /* version */
    5,                                /* headerLen */
    0,                                /* typeOfService */
    0x2A,                             /* totalLen = 42 */
    0,                                /* id */
    0,                                /* flags */
    0,                                /* offset */
    0x40,                             /* timeToLive = 64 */
    0x04,                             /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    { 10, 153, 102, 170},             /* srcAddr = ipSrc */
    {224,   1,   1,   1}              /* dstAddr = ipGrp */
};

/* Packet's payload */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of MC IPv4 packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArrayMcIpv4[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartMcIpv4},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartIpv4},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketMcIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* MC IPv4 PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoMcIpv4 =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                                       /* totalLen */
    sizeof(prvTgfPacketPartArrayMcIpv4) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayMcIpv4                                        /* partsArray */
};

/* Offset of Source IPv4 address */
#define PRV_TGF_SRC_IPV4_OFFSET_CNS    ( TGF_L2_HEADER_SIZE_CNS   +  \
                                         TGF_VLAN_TAG_SIZE_CNS    +  \
                                         TGF_ETHERTYPE_SIZE_CNS   +  \
                                         TGF_IPV4_HEADER_SIZE_CNS -  \
                                         (2 * sizeof(TGF_IPV4_ADDR)) )

/* Offset to store index of modified bit in IPv4 packet*/
#define PRV_TGF_MOD_BIT_IDX_IPV4_OFFSET_CNS ( TGF_L2_HEADER_SIZE_CNS + \
                                         TGF_VLAN_TAG_SIZE_CNS       + \
                                         TGF_ETHERTYPE_SIZE_CNS      + \
                                         TGF_IPV4_HEADER_SIZE_CNS )

/*************************** Restore config ***********************************/

/* Parameters needed to be restored */
static struct
{
    GT_U16 vid;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfIpNonExactMatchRoutingIpv4McPortCountersCheck function
* @endinternal
*
* @brief   Read and check port's counters
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetsCountRx           - Expected number of Rx packets
* @param[in] packetsCountTx           - Expected number of Tx packets
* @param[in] packetSize               - Size of packets
* @param[in] prefixLen                - current prefix length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
static GT_STATUS prvTgfIpNonExactMatchRoutingIpv4McPortCountersCheck
(
    IN CPSS_INTERFACE_INFO_STC *portInterfacePtr,
    IN GT_U32 packetsCountRx,
    IN GT_U32 packetsCountTx,
    IN GT_U32 packetSize,
    IN GT_U32 prefixLen
)
{
    GT_STATUS                     rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_BOOL                       isCountersOk;

    /* Read counters */
    CPSS_TBD_BOOKMARK_EARCH /* casting to GT_U8*/
    rc = prvTgfReadPortCountersEth((GT_U8)portInterfacePtr->devPort.hwDevNum,
                                   portInterfacePtr->devPort.portNum, GT_FALSE, &portCntrs);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfReadPortCountersEth");

    /* Check Tx and Rx counters */
    isCountersOk = portCntrs.goodPktsSent.l[0]   == packetsCountTx &&
                   portCntrs.goodPktsRcv.l[0]    == packetsCountRx &&
                   portCntrs.goodOctetsSent.l[0] == (packetSize + TGF_CRC_LEN_CNS) * packetsCountTx &&
                   portCntrs.goodOctetsRcv.l[0]  == (packetSize + TGF_CRC_LEN_CNS) * packetsCountRx;

    /* Print expected values in case unexpected number of packets is received */
    if (GT_FALSE == isCountersOk)
    {
        PRV_UTF_LOG1_MAC("Unexpected number of packets received for IP prefix length = %u:\n", prefixLen);
        PRV_UTF_LOG1_MAC("Port number = [%u]:\n", portInterfacePtr->devPort.portNum);
        PRV_UTF_LOG2_MAC("  goodPktsSent   = %u, expected value = %u\n",
                portCntrs.goodPktsSent.l[0], packetsCountTx);
        PRV_UTF_LOG2_MAC("  goodPktsRcv    = %u, expected value = %u\n",
                portCntrs.goodPktsRcv.l[0], packetsCountRx);
        PRV_UTF_LOG2_MAC("  goodOctetsSent = %u, expected value = %u\n",
                portCntrs.goodOctetsSent.l[0],((packetSize + TGF_CRC_LEN_CNS) * packetsCountTx));
        PRV_UTF_LOG2_MAC("  goodOctetsRcv  = %u, expected value = %u\n",
                portCntrs.goodOctetsRcv.l[0], ((packetSize + TGF_CRC_LEN_CNS) * packetsCountRx));
    }

    return (GT_FALSE == isCountersOk ? GT_FAIL : rc);
}

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4McCapturedPacketsVerify function
* @endinternal
*
* @brief   Verify that Rx Table contains expected captured packets.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] prefixLen                - current prefix length
* @param[in] defPort                  - TRUE means data from default port shall be verified
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpNonExactMatchRoutingIpv4McCapturedPacketsVerify
(
    IN CPSS_INTERFACE_INFO_STC *portInterfacePtr,
    IN GT_U8 prefixLen,
    IN GT_BOOL defPort
)
{
    GT_STATUS       rc            = GT_OK;
    GT_U32          packetLen     = PRV_TGF_IPV4_PACKET_LEN_CNS;
    GT_U32          origPacketLen = 0;
    GT_BOOL         getFirst      = GT_TRUE;
    GT_BOOL         valid         = GT_FALSE;
    GT_U8           devNum        = 0;
    GT_U8           queue         = 0;
    TGF_NET_DSA_STC rxParam       = {0};
    GT_U8           packetBuf[PRV_TGF_IPV4_PACKET_LEN_CNS];

    /* Go through entries from rxNetworkIf table */
    while (GT_OK == rc)
    {
        packetLen     = PRV_TGF_IPV4_PACKET_LEN_CNS;
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(portInterfacePtr,
                                            TGF_PACKET_TYPE_CAPTURE_E,
                                           getFirst, GT_FALSE, packetBuf,
                                           &packetLen, &origPacketLen,
                                           &devNum, &queue, &rxParam);
        getFirst = GT_FALSE; /* now we get the next */

        /* Validate stored modified bit index value */
        /* AUTODOC: check success criteria: */
        /* AUTODOC:   if modified bit index<prefixLen -> packet is expected on port */
        /* AUTODOC:   if modified bit index>=prefixLen -> packet is not expected */
        if (GT_OK == rc)
        {
            /* Show error log in case unexpected value is received */
            valid =   defPort
                    ? packetBuf[PRV_TGF_MOD_BIT_IDX_IPV4_OFFSET_CNS] >= prefixLen
                    : packetBuf[PRV_TGF_MOD_BIT_IDX_IPV4_OFFSET_CNS] < prefixLen;
            if (valid)
            {
                PRV_UTF_LOG1_MAC("Unexpected packet received on port = %u\n", portInterfacePtr->devPort.portNum);
                PRV_UTF_LOG1_MAC(" Modified bit index = %u\n", packetBuf[PRV_TGF_MOD_BIT_IDX_IPV4_OFFSET_CNS]);
                PRV_UTF_LOG1_MAC(" IP prefix length = %u\n", prefixLen);
                PRV_UTF_LOG4_MAC(" Source IP = %u.%u.%u.%u\n", packetBuf[PRV_TGF_SRC_IPV4_OFFSET_CNS],
                        packetBuf[PRV_TGF_SRC_IPV4_OFFSET_CNS + 1], packetBuf[PRV_TGF_SRC_IPV4_OFFSET_CNS + 2],
                        packetBuf[PRV_TGF_SRC_IPV4_OFFSET_CNS + 3]);
            }
        }
    }

    /* Value GT_NO_MORE is ok in this case */
    return (GT_NO_MORE == rc ? GT_OK : rc);
}

/******************************************************************************\
 *                            Public test functions                           *
\******************************************************************************/

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4McBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4McBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS  rc = GT_OK;
    GT_U8      tagArray[]  = {1, 1, 1, 1};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with tagged ports 0 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 1,2 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_MC_VLANID_CNS,
            (prvTgfPortsArray + 1), NULL, (tagArray + 1), 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 7 with tagged ports 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_DEFAULT_VLANID_CNS,
            prvTgfPortsArray + 3, NULL, (tagArray + 3), 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* Save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum, &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: add FDB entry with MAC 01:00:5e:01:01:01, VLAN 5, VIDX 0 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2PartMcIpv4.daMac,
                                          PRV_TGF_SEND_VLANID_CNS,
                                          PRV_TGF_DEF_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
}

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4McRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4McRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                     rc = GT_OK;
    PRV_TGF_IP_MLL_PAIR_STC       mllPairEntry;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* -------------------------------------------------------------------------
     * 1. Enable routing
     */

    /* AUTODOC: enable IPv4 MC Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 MC Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 2. Setup default configuration
     */

    /* Create default MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));
    mllPairEntry.firstMllNode.mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.firstMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEFAULT_PORT_IDX_CNS];
    mllPairEntry.firstMllNode.nextHopVlanId                    = PRV_TGF_DEFAULT_VLANID_CNS;
    mllPairEntry.firstMllNode.ttlHopLimitThreshold             = 4;
    mllPairEntry.firstMllNode.last                             = GT_TRUE;
    mllPairEntry.nextPointer                                   = 0;

    /* AUTODOC: create 1 default MC Link List 0: */
    /* AUTODOC:   1. port 3, VLAN 7 */
    rc = prvTgfIpMllPairWrite(prvTgfDefMllPointerIndex,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* Create Default MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfDefMllPointerIndex;

    /* AUTODOC: add default MC route entry 1, MLL Ptr 0 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfDefRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");

    /* -------------------------------------------------------------------------
     * 2. Setup test configuration
     */

    /* Create first and second MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));
    mllPairEntry.firstMllNode.mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.firstMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_RCV1_PORT_IDX_CNS];
    mllPairEntry.firstMllNode.nextHopVlanId                    = PRV_TGF_NEXTHOP_MC_VLANID_CNS;
    mllPairEntry.firstMllNode.ttlHopLimitThreshold             = 4;
    mllPairEntry.firstMllNode.last                             = GT_FALSE;

    mllPairEntry.secondMllNode.mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.secondMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_RCV2_PORT_IDX_CNS];
    mllPairEntry.secondMllNode.nextHopVlanId                    = PRV_TGF_NEXTHOP_MC_VLANID_CNS;
    mllPairEntry.secondMllNode.ttlHopLimitThreshold             = 4;
    mllPairEntry.secondMllNode.last                             = GT_TRUE;
    mllPairEntry.nextPointer = 0;

    /* AUTODOC: create 1 and 2 MC Link List 2: */
    /* AUTODOC:   1. port 1, VLAN 6 */
    /* AUTODOC:   2. port 2, VLAN 6 */
    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex,
                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* Create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;

    /* AUTODOC: add default MC route entry 10, MLL Ptr 2 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");
}

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4McTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4McTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portIter    = 0;
    GT_U32    mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;
    GT_U32    prefixIndex = 0;
    GT_U32    exRxCount;
    GT_U32    exTxCount;
    GT_U8     prefixLen;
    GT_U8     mask;
    GT_U8     byteIter;
    GT_U8     bitIter;
    GT_IPADDR srcIp;
    GT_IPADDR grpIp;
    PRV_TGF_IP_LTT_ENTRY_STC mcRouteLttEntry;
    CPSS_INTERFACE_INFO_STC  portInt = {0};
    GT_IPADDR                ipAddrArray[PRV_TGF_BURST_COUNT_CNS];
    TGF_VFD_INFO_STC         vfdArray[2];


    /* AUTODOC: GENERATE TRAFFIC: */

    /* Prepare IP addresses */
    cpssOsMemCpy(srcIp.arIP, prvTgfBaseMcSrcIpv4, sizeof(prvTgfBaseMcSrcIpv4));
    cpssOsMemCpy(grpIp.arIP, prvTgfBaseMcGrpIpv4, sizeof(prvTgfBaseMcGrpIpv4));

    /* Display used prefix */
    PRV_UTF_LOG4_MAC("Used MC SRC IP prefix = %u.%u.%u.%u\n", prvTgfBaseMcSrcIpv4[0],
            prvTgfBaseMcSrcIpv4[1], prvTgfBaseMcSrcIpv4[2], prvTgfBaseMcSrcIpv4[3]);

    /* Setup port interface */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;

    /* Fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* AUTODOC: generate Array of IP addresses from Base IP 10.153.102.170: */
    /* AUTODOC:   get IP from Base IP address by inverting one bit; */
    /* AUTODOC:   inverted bit index is equal to index in IP address array; */
    for (byteIter = 0; byteIter < PRV_TGF_BYTE_ITER__CNS; ++byteIter)
    {
        /* Invert appropriate bit */
        for (bitIter = 0; bitIter < PRV_TGF_BIT_ITER__CNS; ++bitIter)
        {
            cpssOsMemCpy(ipAddrArray[prefixIndex].arIP, prvTgfBaseMcSrcIpv4, sizeof(prvTgfBaseMcSrcIpv4));
            mask = (GT_U8)(1 << (7 - bitIter));
            ipAddrArray[prefixIndex].arIP[byteIter] = (GT_U8)(prvTgfBaseMcSrcIpv4[byteIter] ^ mask);
            ++prefixIndex;
        }
    }

    /* Clear VFD array */
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    /* Prepare VFD array entry for DST IP address*/
    vfdArray[0].mode               = TGF_VFD_MODE_ARRAY_PATTERN_E;
    vfdArray[0].arrayPatternOffset = 0;
    vfdArray[0].offset             = PRV_TGF_SRC_IPV4_OFFSET_CNS;
    vfdArray[0].arrayPatternSize   = sizeof(GT_IPADDR);
    vfdArray[0].cycleCount         = sizeof(GT_IPADDR);
    vfdArray[0].arrayPatternPtr    = (GT_U8*)ipAddrArray;

    /* Prepare VFD array entry to transfer modified bit index */
    vfdArray[1].mode       = TGF_VFD_MODE_INCREMENT_E;
    vfdArray[1].incValue   = 1;
    vfdArray[1].offset     = PRV_TGF_MOD_BIT_IDX_IPV4_OFFSET_CNS;
    vfdArray[1].cycleCount = 1;

    /* Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoMcIpv4,
                             PRV_TGF_BURST_COUNT_CNS, 2, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 10);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

   /* set default MLL counters mode */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    
    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Iterate trough the all prefix lengths [1..31]: */
    for (prefixLen = 1; prefixLen < PRV_TGF_MAX_SRC_PREFIX_LEN_CNS; ++prefixLen)
    {
        /* AUTODOC: add IPv4 MC prefix 10.153.102.170/[1..31] the LPM DB */
        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, prvTgfVrId,
                                       grpIp, PRV_TGF_MAX_GRP_PREFIX_LEN_CNS,
                                       srcIp, prefixLen,
                                       &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);       

        /* Reset Ethernet counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");     

        /* Clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* Enable capturing on MC receive port 1 */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV1_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
               prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_RCV1_PORT_IDX_CNS]);

        /* Enable capturing on MC receive port 2 */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV2_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
               prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_RCV2_PORT_IDX_CNS]);

        /* Enable capturing on default MC port */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEFAULT_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
               prvTgfDevNum, prvTgfPortsArray[PRV_TGF_DEFAULT_PORT_IDX_CNS]);

        /* Disable packet tracing */
        rc = tgfTrafficTracePacketByteSet(GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTracePacketByteSet: %d", prvTgfDevNum);

        /* AUTODOC: for each prefix send 32 IPv4 packets from port 0 with: */
        /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:02 */
        /* AUTODOC:   VID=5, srcIP=<generated IP Array>, dstIPs=224.1.1.1 */
        /* AUTODOC:   1-st byte in Payload contains modified bit index value */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        if (GT_TRUE == prvUtfIsGmCompilation())
            cpssOsTimerWkAfter(2000);

        /* Disable capturing on default MC port */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEFAULT_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
               prvTgfDevNum, prvTgfPortsArray[PRV_TGF_DEFAULT_PORT_IDX_CNS]);

        /* Disable capturing on MC receive port 2 */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV2_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
               prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_RCV2_PORT_IDX_CNS]);

        /* Disable capturing on MC receive port 1 */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV1_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
               prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_RCV1_PORT_IDX_CNS]);

        /* Check port counters to verify MC ports contain expected number of packets */
        exRxCount = PRV_TGF_BURST_COUNT_CNS - prefixLen;
        exTxCount = PRV_TGF_BURST_COUNT_CNS - prefixLen;
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV1_PORT_IDX_CNS];

        /* AUTODOC: check received packets num on port 1 to be 32-PrefixLen */
        rc = prvTgfIpNonExactMatchRoutingIpv4McPortCountersCheck(&portInt, exRxCount, exTxCount,
                prvTgfPacketInfoMcIpv4.totalLen, prefixLen);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortCountersCheck");

        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV2_PORT_IDX_CNS];

        /* AUTODOC: check received packets num on port 2 to be 32-PrefixLen */
        rc = prvTgfIpNonExactMatchRoutingIpv4McPortCountersCheck(&portInt, exRxCount, exTxCount,
                prvTgfPacketInfoMcIpv4.totalLen, prefixLen);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortCountersCheck");

        /* Check port counters to verify default MC port contains expected number of packets */
        exRxCount = PRV_TGF_BURST_COUNT_CNS - exRxCount;
        exTxCount = PRV_TGF_BURST_COUNT_CNS - exTxCount;
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEFAULT_PORT_IDX_CNS];

        /* AUTODOC: check received packets num on default port 3 to be 32-PrefixLen */
        rc = prvTgfIpNonExactMatchRoutingIpv4McPortCountersCheck(&portInt, exRxCount, exTxCount,
                prvTgfPacketInfoMcIpv4.totalLen, prefixLen);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortCountersCheck");

        /* AUTODOC: Verify captured packets on port 1: */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV1_PORT_IDX_CNS];

        rc = prvTgfIpNonExactMatchRoutingIpv4McCapturedPacketsVerify(&portInt, prefixLen, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCapturedPacketsVerify");

        /* AUTODOC: Verify captured packets on port 2: */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_RCV2_PORT_IDX_CNS];
        rc = prvTgfIpNonExactMatchRoutingIpv4McCapturedPacketsVerify(&portInt, prefixLen, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCapturedPacketsVerify");

        /* Verify captured packets on default port */
        portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEFAULT_PORT_IDX_CNS];

        /* AUTODOC: Verify captured packets on default port 3: */
        rc = prvTgfIpNonExactMatchRoutingIpv4McCapturedPacketsVerify(&portInt, prefixLen, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCapturedPacketsVerify");

        /* AUTODOC: delete the MC Ipv4 prefix */
        rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, prvTgfVrId, grpIp,
                PRV_TGF_MAX_GRP_PREFIX_LEN_CNS, srcIp, prefixLen);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryDel: %d", prvTgfDevNum);
    }

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpNonExactMatchRoutingIpv4McConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore Route Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID prvTgfIpNonExactMatchRoutingIpv4McConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* AUTODOC: flush all MC Ipv4 prefixes */
    rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, prvTgfVrId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush");

    /* AUTODOC: disable multicast IPv4 routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
                                   CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable multicast IPv4 routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

    /* Invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* Invalidate VLAN entry for MC subscribers */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_MC_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 1");

    /* invalidate default VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_DEFAULT_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 2");

    /* set default MLL counters mode */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    
    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");
 
    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");
}


