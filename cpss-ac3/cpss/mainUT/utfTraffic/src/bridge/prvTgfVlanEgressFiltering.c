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
* @file prvTgfVlanEgressFiltering.c
*
* @brief VLAN Egress Filtering Mirror to Analyzer
*
* @version   12
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
#include <common/tgfMirror.h>
#include <bridge/prvTgfVlanEgressFiltering.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS          2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* analyzer port num */
#define PRV_TGF_ANALYZER_PORT_IDX_CNS 3

/* default VLAN ID */
#define PRV_TGF_DEF_VLAN_ID_CNS       2

/* invalid VLAN ID */
#define PRV_TGF_INV_VLAN_ID_CNS       3

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 5;

/* VLAN Id array */
static GT_U8        prvTgfVlanIdArray[] = {PRV_TGF_DEF_VLAN_ID_CNS, PRV_TGF_INV_VLAN_ID_CNS};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                               {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/* Data of packet */
static GT_U8 prvTgfPayloadDataArr[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                       0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                       0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                                       0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {sizeof(prvTgfPayloadDataArr), prvTgfPayloadDataArr};

/* Parts of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {{TGF_PACKET_PART_L2_E, &prvTgfPacketL2Part},
                                                      {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}};
/******************************************************************************/

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U8                       analyzerPort;
    GT_U8                       analyzerdev;
    GT_BOOL                     mirrorEnable;
    GT_U16                      defVlanId;
    GT_BOOL                     egrFltEnable;
} prvTgfRestoreCfg;

/******************************************************************************/



/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanEgrFltConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgVlanEgrFltConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                             isTagged  = GT_FALSE;
    GT_U32                              portIter  = 0;
    GT_U32                              vlanIter  = 0;
    GT_U32                              portCount = 0;
    GT_U32                              vlanCount = 0;
    GT_BOOL                             enable    = GT_FALSE;
    GT_STATUS                           rc        = GT_OK;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    PRV_TGF_BRG_MAC_ENTRY_STC           macEntry;
    GT_U32                              index;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

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
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.vrfId                = 0;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] =
            (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsTagging,prvTgfPortsArray[portIter]))
                                    ? PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E
                                    : PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* get vlan count */
    vlanCount = sizeof(prvTgfVlanIdArray) / sizeof(prvTgfVlanIdArray[0]);

    /* AUTODOC: create VLAN 2 with ports 0, 1 */
    /* AUTODOC: create VLAN 3 with ports 2, 3 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, prvTgfVlanIdArray[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanIdArray[vlanIter]);
    }

    /* add ports to vlan member */
    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        isTagged = GT_TRUE;

        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanIdArray[portIter / (portCount / 2)],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanIdArray[portIter / (portCount / 2)],
                                     prvTgfPortsArray[portIter], isTagged);
    }

    /* clear entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* configure FDB entry */
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(prvTgfPacketL2Part.daMac));

    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    macEntry.isStatic  = GT_TRUE;
    macEntry.daCommand = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute   = GT_FALSE;
    macEntry.sourceId  = 0;
    macEntry.userDefined = 0;
    macEntry.daQosIndex  = 0;
    macEntry.saQosIndex  = 0;
    macEntry.daSecurityLevel = 0;
    macEntry.saSecurityLevel = 0;
    macEntry.appSpecificCpuCode = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLANs 2, port 3 */
    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLANs 3, port 3 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* set Vlan ID */
        macEntry.key.key.macVlan.vlanId = prvTgfVlanIdArray[vlanIter];

        /* set FDB entry */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet: %d", prvTgfDevNum);
    }

    /* save def values */
    rc = prvTgfMirrorRxAnalyzerPortGet(prvTgfDevNum, &(prvTgfRestoreCfg.analyzerPort), &(prvTgfRestoreCfg.analyzerdev));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxAnalyzerPortGet: %d", prvTgfDevNum);

    /* AUTODOC: set port 3 to analyzer port */
    rc = prvTgfMirrorRxAnalyzerPortSet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS], prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxAnalyzerPortSet: %d", prvTgfDevNum);

    /* save def values */
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.mirrorEnable), &index);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set port 0 to be mirrored */
    enable = GT_TRUE;

    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);

    /* save def values */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &(prvTgfRestoreCfg.defVlanId));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: set PVID to 2 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_DEF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_DEF_VLAN_ID_CNS);

    /* save def VLAN egress filtering state */
    rc = prvTgfBrgVlanEgressFilteringEnableGet(prvTgfDevNum, &(prvTgfRestoreCfg.egrFltEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressFilteringEnableGet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgVlanEgrFltTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgVlanEgrFltTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portsCount = 0;
    GT_U32          partsCount = 0;
    GT_U32          packetSize = 0;
    GT_U32          portIter,i = 0;
    GT_U32          numVfd     = 0;
    GT_BOOL         enable     = GT_FALSE;
    TGF_PACKET_STC  packetInfoPtr;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfoPtr.totalLen   = packetSize;
    packetInfoPtr.numOfParts = partsCount;
    packetInfoPtr.partsArray = prvTgfPacketPartArray;

    /* AUTODOC: Make 2 iterations: */
    for (i = 0; i < PRV_TGF_MAX_ITER_CNS; i++)
    {
        /* AUTODOC: enable egress filtering */
        enable = GT_TRUE;

        rc = prvTgfBrgVlanEgressFilteringEnable(enable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressFilteringEnable: %d, %d,",
                                     prvTgfDevNum, enable);

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending INVALID unknown unicast pakets to port [%d] -------\n",
                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send packet with DA 00:00:00:00:00:02 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get traffic on port 3 */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* check counters */
            switch (portIter)
            {
                case PRV_TGF_SEND_PORT_IDX_CNS:
                    /* check Rx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                                 "get another goodOctetsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                                 "get another goodPktsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                                 "get another ucPktsRcv counter than expected");
                    break;

                case PRV_TGF_ANALYZER_PORT_IDX_CNS:
                    /* check Tx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                                 "get another goodOctetsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                                 "get another goodPktsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                                 "get another ucPktsSent counter than expected");
                    break;

                default:
                    /* check Rx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0],
                                                 "get another goodOctetsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                                 "get another goodPktsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                                 "get another ucPktsRcv counter than expected");

                    /* check Tx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0],
                                                 "get another goodOctetsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                                 "get another goodPktsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                                 "get another ucPktsSent counter than expected");
            }
        }

        /* AUTODOC: disable egress filtering */
        enable = GT_FALSE;

        rc = prvTgfBrgVlanEgressFilteringEnable(enable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressFilteringEnable: %d, %d,",
                                     prvTgfDevNum, enable);

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending VALID unknown unicast pakets to port [%d] -------\n",
                         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send packet with DA 00:00:00:00:00:02 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get traffic on port 3 */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* check counters */
            switch (portIter)
            {
                case PRV_TGF_SEND_PORT_IDX_CNS:
                    /* check Rx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                                 "get another goodOctetsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                                 "get another goodPktsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                                 "get another ucPktsRcv counter than expected");
                    break;

                case PRV_TGF_ANALYZER_PORT_IDX_CNS:
                    /* check Tx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount * 2, portCntrs.goodOctetsSent.l[0],
                                                 "get another goodOctetsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsSent.l[0],
                                                 "get another goodPktsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.ucPktsSent.l[0],
                                                 "get another ucPktsSent counter than expected");
                    break;

                default:
                    /* check Rx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0],
                                                 "get another goodOctetsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                                 "get another goodPktsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                                 "get another ucPktsRcv counter than expected");

                    /* check Tx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0],
                                                 "get another goodOctetsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                                 "get another goodPktsSent counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                                 "get another ucPktsSent counter than expected");
            }
        }
    }
}

/**
* @internal prvTgfBrgVlanEgrFltConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanEgrFltConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = 0;
    GT_STATUS   rc        = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* get vlan count */
    vlanCount = sizeof(prvTgfVlanIdArray) / sizeof (prvTgfVlanIdArray[0]);

    /* invalidate vlan entries */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanIdArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanIdArray[vlanIter]);
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore def Rx analyzer port */
    rc = prvTgfMirrorRxAnalyzerPortSet(prvTgfRestoreCfg.analyzerPort, prvTgfRestoreCfg.analyzerdev);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxAnalyzerPortSet: %d", prvTgfDevNum);

    /* AUTODOC: restore def Rx mirrored port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.mirrorEnable, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.mirrorEnable);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.defVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.defVlanId);

    /* AUTODOC: restore def VLAN egress filtering state */
    rc = prvTgfBrgVlanEgressFilteringEnable(prvTgfRestoreCfg.egrFltEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressFilteringEnable: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.egrFltEnable);
}


