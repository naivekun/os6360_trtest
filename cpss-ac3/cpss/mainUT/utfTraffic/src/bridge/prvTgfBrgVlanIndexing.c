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
* @file prvTgfBrgVlanIndexing.c
*
* @brief VLAN Indexing
*
* @version   4
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <port/prvTgfPortFWS.h>
#include <bridge/prvTgfBrgVlanIndexing.h>
#include <common/tgfCommon.h>
#include <utf/private/prvUtfExtras.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  13

#define VLANID_PORT0_CNS 2
#define VLANID_PORT1_CNS 3

#define VLANID_STG_CNS 14

/* TAG1 VLAN Ids */
#define PRV_TGF_VLANID1_CNS 178

#define VLANID1_PORT0_CNS 1
#define VLANID1_PORT1_CNS 3

#define VLANID1_STG_CNS 179

#define PRV_TGF_VLANID2_CNS 233

#define VLANID2_PORT0_CNS 2
#define VLANID2_PORT1_CNS 0

#define VLANID2_STG_CNS 234

/* default number of packets to send */
static GT_U32       prvTgfBurstCount   = 1;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},               /* srcMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}                /* dstMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* Second VLAN_TAG parts */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID1_CNS                           /* pri, cfi, VlanId */
};

static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag2Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID2_CNS                           /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/************************ Double tagged packet ********************************/

/* PARTS of double tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketDoubleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfPacketDoubleTag2PartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of double tagged packet */
#define PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of double tagged packet with CRC */
#define PRV_TGF_PACKET_DOUBLE_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Double tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoDoubleTag =
{
    PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketDoubleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketDoubleTagPartArray                                        /* partsArray */
};

static TGF_PACKET_STC prvTgfPacketInfoDoubleTag2 =
{
    PRV_TGF_PACKET_DOUBLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketDoubleTag2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketDoubleTag2PartArray                                        /* partsArray */
};

/* expected packet parts */
static TGF_PACKET_PART_STC expectedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

#define VLAN_TAG_INDEX_CNS  1
#define VID1_TAG_INDEX_CNS  2

/* expected packet info */
static TGF_PACKET_STC expectedPacket =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,      /* totalLen */
    sizeof(expectedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    expectedPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* save Member table indexing mode for restore */
static PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT prvTgfMemberIndexingModeForRestore[2];

/* save STG indexing mode for restore */
static PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT prvTgfStgIndexingModeForRestore[2];

/* save ports Span State for restore */
static CPSS_STP_STATE_ENT prvTgfPortsSpanStateForRestore[2];

/* save port Tag State for restore */
static PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_ENT prvTgfPortTagStateForRestore;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgBrgVlanIndexing2PortsVlanCreate function
* @endinternal
*
* @brief   Set VLAN entry with 2 selected member ports, stgId & tag command.
*
* @param[in] vlanId                   -  to be configured
* @param[in] memberPort0              - member port in vlan (iterator number)
* @param[in] memberPort1              - additional member port in vlan (iterator number)
* @param[in] stgId                    - span state group index
* @param[in] tagCmd                   - VLAN tag command
*                                       None
*/
static GT_VOID prvTgfBrgBrgVlanIndexing2PortsVlanCreate
(
    IN GT_U16                            vlanId,
    IN GT_U32                            memberPort0,
    IN GT_U32                            memberPort1,
    IN GT_U32                            stgId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd
)
{
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set vlan entry */
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
    vlanInfo.stgId                = stgId;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[memberPort0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[memberPort1]);

    portsTaggingCmd.portsCmd[prvTgfPortsArray[memberPort0]] = tagCmd;
    portsTaggingCmd.portsCmd[prvTgfPortsArray[memberPort1]] = tagCmd;

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfBrgBrgVlanIndexing4PortsVlanCreate function
* @endinternal
*
* @brief   Set VLAN entry with all 4 member ports, stgId & tag command.
*
* @param[in] vlanId                   -  to be configured
* @param[in] stgId                    - span state group index
* @param[in] tagCmd                   - VLAN tag command
*                                       None
*/
static GT_VOID prvTgfBrgBrgVlanIndexing4PortsVlanCreate
(
    IN GT_U16                            vlanId,
    IN GT_U32                            stgId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd
)
{
    GT_STATUS                   rc        = GT_OK;
    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set vlan entry */
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
    vlanInfo.stgId                = stgId;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = tagCmd;
    }

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfBrgBrgVlanIndexingTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfBrgBrgVlanIndexingTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfBrgBrgVlanIndexingTestTableReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*/
static GT_VOID prvTgfBrgBrgVlanIndexingTestTableReset
(
    GT_VOID
)
{
    GT_STATUS   rc      = GT_OK;


    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate vlan entries (and reset vlans entries) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);


    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID1_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID2_CNS);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgBrgVlanMembersIndexingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgBrgVlanMembersIndexingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: vlan 13  - port members 2 & 3 */
    /* AUTODOC: vlan 178 - port members 1 & 3 */
    /* AUTODOC: vlan 233 - port members 0 & 2 */

    prvTgfBrgBrgVlanIndexing2PortsVlanCreate(PRV_TGF_VLANID_CNS,
                                             VLANID_PORT0_CNS,
                                             VLANID_PORT1_CNS,
                                             0,
                                     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    prvTgfBrgBrgVlanIndexing2PortsVlanCreate(PRV_TGF_VLANID1_CNS,
                                             VLANID1_PORT0_CNS,
                                             VLANID1_PORT1_CNS,
                                             0,
                                     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    prvTgfBrgBrgVlanIndexing2PortsVlanCreate(PRV_TGF_VLANID2_CNS,
                                             VLANID2_PORT0_CNS,
                                             VLANID2_PORT1_CNS,
                                             0,
                                     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    /* AUTODOC: Save Members table indexing mode for restore. */
    rc = prvTgfBrgVlanMembersTableIndexingModeGet(CPSS_DIRECTION_INGRESS_E,
                                                  &prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeGet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);

    rc = prvTgfBrgVlanMembersTableIndexingModeGet(CPSS_DIRECTION_EGRESS_E,
                                                  &prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeGet: %d, %d",
                                 CPSS_DIRECTION_EGRESS_E,
                                 prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);
}

/**
* @internal prvTgfBrgBrgVlanMembersIndexingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgBrgVlanMembersIndexingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[VLANID_PORT1_CNS];


    /* AUTODOC: configure INGRESS & EGRESS VLAN Members indexing by VLAN */
    rc = prvTgfBrgVlanMembersTableIndexingModeSet(CPSS_DIRECTION_BOTH_E,
                                                  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_BOTH_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);

    /* AUTODOC: Send packet on vlan 13, port 2. */
    /* AUTODOC: Expect forwarding to port 3.    */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (VLANID_PORT1_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expectedon port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: enable ingress vlan filtering on port 2 */
    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS], GT_TRUE);

    /* AUTODOC: Send packet on vlan 13, port 2. */
    /* AUTODOC: Expect forwarding to port 3.    */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (VLANID_PORT1_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expectedon port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: configure INGRESS VLAN Members indexing by VID1 */
    rc = prvTgfBrgVlanMembersTableIndexingModeSet(CPSS_DIRECTION_INGRESS_E,
                                                  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);

    /* AUTODOC: Send packet on vlan 13, vid1 178, port 2. */
    /* AUTODOC: Expect NO forwarding.                     */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                        "get another goodPktsSent counter than expectedon port %d",
                                        prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: configure INGRESS VLAN Members (back to) indexing by VLAN */
    rc = prvTgfBrgVlanMembersTableIndexingModeSet(CPSS_DIRECTION_INGRESS_E,
                                                  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);

    /* AUTODOC: disable ingress vlan filtering on port 2 */
    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS], GT_FALSE);

    /* AUTODOC: Send packet on vlan 13, vid1 233, port 2. */
    /* AUTODOC: Expect forwarding to port 3.              */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (VLANID_PORT1_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expectedon port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: configure EGRESS VLAN Members indexing by VID1 */
    rc = prvTgfBrgVlanMembersTableIndexingModeSet(CPSS_DIRECTION_EGRESS_E,
                                                  PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_EGRESS_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);

    /* AUTODOC: Send packet on vlan 13, vid1 233, port 2. */
    /* AUTODOC: Expect forwarding to port 0.              */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag2);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (VLANID2_PORT1_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expectedon port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }
}

/**
* @internal prvTgfBrgBrgVlanMembersIndexingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBrgVlanMembersIndexingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Restore Members table indexing mode. */
    rc = prvTgfBrgVlanMembersTableIndexingModeSet(CPSS_DIRECTION_INGRESS_E,
                                                  prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);

    rc = prvTgfBrgVlanMembersTableIndexingModeSet(CPSS_DIRECTION_EGRESS_E,
                                                  prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMembersTableIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_EGRESS_E,
                                 prvTgfMemberIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);

    prvTgfBrgBrgVlanIndexingTestTableReset();
}

/**
* @internal prvTgfBrgBrgVlanStgIndexingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgBrgVlanStgIndexingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: all vlans - port members 0, 1, 2 & 3 */
    prvTgfBrgBrgVlanIndexing4PortsVlanCreate(PRV_TGF_VLANID_CNS,
                                             VLANID_STG_CNS,
                                     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    prvTgfBrgBrgVlanIndexing4PortsVlanCreate(PRV_TGF_VLANID1_CNS,
                                             VLANID1_STG_CNS,
                                     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    prvTgfBrgBrgVlanIndexing4PortsVlanCreate(PRV_TGF_VLANID2_CNS,
                                             VLANID2_STG_CNS,
                                     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    /* AUTODOC: Save STG indexing mode for restore */
    rc = prvTgfBrgVlanStgIndexingModeGet(CPSS_DIRECTION_INGRESS_E,
                                         &prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeGet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);

    rc = prvTgfBrgVlanStgIndexingModeGet(CPSS_DIRECTION_EGRESS_E,
                                         &prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeGet: %d, %d",
                                 CPSS_DIRECTION_EGRESS_E,
                                 prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);

    /* AUTODOC: Save STP for restore */
    rc = prvTgfBrgStpPortStateGet(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS],
                                  VLANID1_STG_CNS, &prvTgfPortsSpanStateForRestore[0]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateGet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS],
                                 VLANID1_STG_CNS, prvTgfPortsSpanStateForRestore[0]);

    rc = prvTgfBrgStpPortStateGet(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT1_CNS],
                                  VLANID2_STG_CNS, &prvTgfPortsSpanStateForRestore[1]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateGet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[VLANID_PORT1_CNS],
                                 VLANID2_STG_CNS, prvTgfPortsSpanStateForRestore[1]);

    /* AUTODOC: STP state:  */
    /* AUTODOC: port      | 0 | 1 | 2 | 3 | */
    /* AUTODOC: vlan 13   | D | D | D | D | */
    /* AUTODOC: vlan 178  | D | D | B | D | */
    /* AUTODOC: vlan 233  | D | D | D | B | */
    /* Configure STP tables */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS],
                                  VLANID1_STG_CNS, CPSS_STP_BLCK_LSTN_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d, srg: %d",
                                 prvTgfDevNum, VLANID1_STG_CNS);

    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT1_CNS],
                                  VLANID2_STG_CNS, CPSS_STP_BLCK_LSTN_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d, srg: %d",
                                 prvTgfDevNum, VLANID2_STG_CNS);
}

/**
* @internal prvTgfBrgBrgVlanStgIndexingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgBrgVlanStgIndexingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[VLANID_PORT1_CNS];

    /* AUTODOC: configure INGRESS & EGRESS VLAN STG indexing by VLAN */
    rc = prvTgfBrgVlanStgIndexingModeSet(CPSS_DIRECTION_BOTH_E,
                                         PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_BOTH_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);

    /* AUTODOC: Send packet on vlan 13, port 2.           */
    /* AUTODOC: Expect forwarding to (all) port 0,1 & 3.  */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: configure INGRESS VLAN STG indexing by VID1 */
    rc = prvTgfBrgVlanStgIndexingModeSet(CPSS_DIRECTION_INGRESS_E,
                                         PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);

    /* AUTODOC: Send packet on vlan 13, vid1 178, port 2.   */
    /* AUTODOC: Expect NO forwarding.                       */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expectedon port %d",
                                     prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Send packet on vlan 13, vid1 233, port 2. */
    /* AUTODOC: Expect forwarding to (all) port 0,1 & 3.  */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag2);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expectedon port %d",
                                     prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: configure INGRESS VLAN STG (back to) indexing by VLAN */
    rc = prvTgfBrgVlanStgIndexingModeSet(CPSS_DIRECTION_INGRESS_E,
                                         PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);

    /* AUTODOC: configure EGRESS VLAN STG indexing by VID1 */
    rc = prvTgfBrgVlanStgIndexingModeSet(CPSS_DIRECTION_EGRESS_E,
                                         PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_EGRESS_E,
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);

    /* AUTODOC: Send packet on vlan 13, vid1 178, port 2. */
    /* AUTODOC: Expect forwarding to (all) port 0,1 & 3.  */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expectedon port %d",
                                     prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Send packet on vlan 13, vid1 233, port 2.  */
    /* AUTODOC: Expect forwarding to port 0 & 1 only.      */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag2);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (VLANID_PORT1_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expectedon port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }
}

/**
* @internal prvTgfBrgBrgVlanStgIndexingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBrgVlanStgIndexingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Restore STG indexing mode. */
    rc = prvTgfBrgVlanStgIndexingModeSet(CPSS_DIRECTION_INGRESS_E,
                                         prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_INGRESS_E,
                                 prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_INGRESS_E]);

    rc = prvTgfBrgVlanStgIndexingModeSet(CPSS_DIRECTION_EGRESS_E,
                                         prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIndexingModeSet: %d, %d",
                                 CPSS_DIRECTION_EGRESS_E,
                                 prvTgfStgIndexingModeForRestore[CPSS_DIRECTION_EGRESS_E]);

    /* AUTODOC: Restore STP */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS],
                                  VLANID1_STG_CNS, prvTgfPortsSpanStateForRestore[0]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[VLANID_PORT0_CNS],
                                 VLANID1_STG_CNS, prvTgfPortsSpanStateForRestore[0]);

    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[VLANID_PORT1_CNS],
                                  VLANID2_STG_CNS, prvTgfPortsSpanStateForRestore[1]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d, %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[VLANID_PORT1_CNS],
                                 VLANID2_STG_CNS, prvTgfPortsSpanStateForRestore[1]);

    prvTgfBrgBrgVlanIndexingTestTableReset();
}

/**
* @internal prvTgfBrgBrgVlanTagStateIndexingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfBrgBrgVlanTagStateIndexingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: Save Port Tag State for restore */
    rc = prvTgfBrgVlanTagStateIndexingModeGet(prvTgfDevNum,
                                              prvTgfPortsArray[VLANID_PORT1_CNS],
                                              &prvTgfPortTagStateForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagStateIndexingModeGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[VLANID_PORT1_CNS],
                                 prvTgfPortTagStateForRestore);

    /* AUTODOC: all vlans - all port members 0, 1, 2 & 3 */
    /* AUTODOC: vlan 13   - egress tag state: double tagged  */
    /* AUTODOC: vlan 178  - egress tag state: untagged       */

    prvTgfBrgBrgVlanIndexing4PortsVlanCreate(PRV_TGF_VLANID_CNS,
                                             0,
                                     PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);

    prvTgfBrgBrgVlanIndexing4PortsVlanCreate(PRV_TGF_VLANID1_CNS,
                                             0,
                                     PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E);

}

/**
* @internal prvTgfBrgBrgVlanTagStateIndexingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgBrgVlanTagStateIndexingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/

    /* AUTODOC: GENERATE TRAFFIC: */

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[VLANID_PORT1_CNS];

    /* AUTODOC: configure port 3 vlan Tag State indexing by VLAN */
    rc = prvTgfBrgVlanTagStateIndexingModeSet(prvTgfDevNum,
                                              prvTgfPortsArray[VLANID_PORT1_CNS],
                                              PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagStateIndexingModeSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[VLANID_PORT1_CNS],
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VLAN_E);

    /* AUTODOC: Send packet on vlan 13, port 2.                */
    /* AUTODOC: Expect packet double tagged on ports 0,1 & 3.  */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected on port %d",
                                     prvTgfPortsArray[portIter]);

        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_PACKET_DOUBLE_TAG_CRC_LEN_CNS, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected on port %d",
                                     prvTgfPortsArray[portIter]);

        if (VLANID_PORT1_CNS == portIter)
        {
            /* print captured packets and check TriggerCounters */
            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                    &portInterface,
                    &expectedPacket,
                    prvTgfBurstCount,/*numOfPackets*/
                    0/*vfdNum*/,
                    NULL /*vfdArray*/,
                    NULL, /* bytesNum's skip list */
                    0,    /* length of skip list */
                    &actualCapturedNumOfPackets,
                    NULL/*onFirstPacketNumTriggersBmpPtr*/);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     "port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
        }
    }

    /* AUTODOC: configure port 3 vlan Tag State indexing by VID1 */
    rc = prvTgfBrgVlanTagStateIndexingModeSet(prvTgfDevNum,
                                              prvTgfPortsArray[VLANID_PORT1_CNS],
                                              PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagStateIndexingModeSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[VLANID_PORT1_CNS],
                                 PRV_TGF_BRG_VLAN_TBL_INDEX_MODE_VID1_E);

    /* AUTODOC: Send packet on vlan 13, vid1 178, port 2.                       */
    /* AUTODOC: Expect packet double tagged on ports 0 & 1, untagged on port 3. */
    prvTgfBrgBrgVlanIndexingTestPacketSend(&portInterface, &prvTgfPacketInfoDoubleTag);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (VLANID_PORT0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected on port %d",
                                     prvTgfPortsArray[portIter]);

        if(VLANID_PORT1_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_PACKET_DOUBLE_TAG_CRC_LEN_CNS - 2 * TGF_VLAN_TAG_SIZE_CNS,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);

            expectedPacket.partsArray[VLAN_TAG_INDEX_CNS].type =
                TGF_PACKET_PART_SKIP_E;
            expectedPacket.partsArray[VID1_TAG_INDEX_CNS].type =
                TGF_PACKET_PART_SKIP_E;

            /* print captured packets and check TriggerCounters */
            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                    &portInterface,
                    &expectedPacket,
                    prvTgfBurstCount,/*numOfPackets*/
                    0/*vfdNum*/,
                    NULL /*vfdArray*/,
                    NULL, /* bytesNum's skip list */
                    0,    /* length of skip list */
                    &actualCapturedNumOfPackets,
                    NULL/*onFirstPacketNumTriggersBmpPtr*/);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     "port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

            expectedPacket.partsArray[VLAN_TAG_INDEX_CNS].type =
                TGF_PACKET_PART_VLAN_TAG_E;
            expectedPacket.partsArray[VID1_TAG_INDEX_CNS].type =
                TGF_PACKET_PART_VLAN_TAG_E;
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_PACKET_DOUBLE_TAG_CRC_LEN_CNS,
                                         portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }
}

/**
* @internal prvTgfBrgBrgVlanTagStateIndexingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgBrgVlanTagStateIndexingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Restore Port Tag State */
    rc = prvTgfBrgVlanTagStateIndexingModeSet(prvTgfDevNum,
                                              prvTgfPortsArray[VLANID_PORT1_CNS],
                                              prvTgfPortTagStateForRestore);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTagStateIndexingModeSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[VLANID_PORT1_CNS],
                                 prvTgfPortTagStateForRestore);

    prvTgfBrgBrgVlanIndexingTestTableReset();
}

