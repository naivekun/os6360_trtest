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
* @file prvTgfPortMacCountersCapture.c
*
* @brief Specific Port MAC Counters features testing
*
* @version   17
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>


#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <port/prvTgfPortMacCountersCapture.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>

#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#else
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#endif

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            2

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_0_CNS     0

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_2_CNS     2

/* default number of packets to send */
static GT_U32   prvTgfBurstCount   = 1;

/* FDB MAC entry */
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;


/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x22}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55};

/* DATA of packet with size 100 */
static GT_U8 prvTgfPayloadData80BytesArr[80];

/* DATA of packet with size 200 */
static GT_U8 prvTgfPayloadData200BytesArr[200];

/* DATA of packet with size 300 */
static GT_U8 prvTgfPayloadData300BytesArr[300];

/* DATA of packet with size 500 */
static GT_U8 prvTgfPayloadData500BytesArr[500];

/* DATA of packet with size 1024 */
static GT_U8 prvTgfPayloadData1024BytesArr[_1KB];

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)


/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static GT_VOID prvTgfPortMacCounterPacketSend
(
    IN  GT_U32                        portNum,
    IN  TGF_PACKET_STC               *packetInfoPtr
);

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfPortMacCountersFdbConfigurationSet function
* @endinternal
*
* @brief   Set FDB configuration.
*         MAC         VID   VIDX  DA_CMD
*         01:02:03:04:05:06  2    1    CPSS_MAC_TABLE_FRWRD_E
*/
static GT_VOID prvTgfPortMacCountersFdbConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc;                 /* return code */

    /* Add 5 MAC Entries */

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.dstInterface.type            = CPSS_INTERFACE_VIDX_E;
    prvTgfMacEntry.dstInterface.vidx            = 1;
    prvTgfMacEntry.isStatic                     = GT_TRUE;
    prvTgfMacEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.daRoute                      = GT_FALSE;
    prvTgfMacEntry.mirrorToRxAnalyzerPortEn     = GT_FALSE;
    prvTgfMacEntry.userDefined                  = 0;
    prvTgfMacEntry.daQosIndex                   = 0;
    prvTgfMacEntry.saQosIndex                   = 0;
    prvTgfMacEntry.daSecurityLevel              = 0;
    prvTgfMacEntry.saSecurityLevel              = 0;
    prvTgfMacEntry.appSpecificCpuCode           = GT_FALSE;
    prvTgfMacEntry.pwId                         = 0;
    prvTgfMacEntry.spUnknown                    = GT_FALSE;
    prvTgfMacEntry.sourceId                     = 1;

    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[0] = 0x01;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[1] = 0x02;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[2] = 0x03;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[3] = 0x04;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[4] = 0x05;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[5] = 0x06;

    /* AUTODOC: add FDB entry with MAC 01:02:03:04:05:06, VLAN 2, VIDX 1 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in prvTgfBrgFdbMacEntrySet %d", GT_TRUE);
}

/**
* @internal prvTgfPortMacCountersFdbEntryDelete function
* @endinternal
*
* @brief   Delete FDB entry.
*/
static GT_VOID prvTgfPortMacCountersFdbEntryDelete
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: delete FDB entry */
    rc = prvTgfBrgFdbMacEntryDelete(&(prvTgfMacEntry.key));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in prvTgfBrgFdbMacEntrySet %d", GT_TRUE);
}

/**
* @internal prvTgfPortMacCountersVlanConfigurationSet function
* @endinternal
*
* @brief   Set Default Vlan Port MAC Counters configuration.
*/
GT_VOID prvTgfPortMacCountersVlanConfigurationSet
(
    GT_VOID
)
{

    GT_STATUS rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPortMacCountersToCpuVlanConfigurationSet function
* @endinternal
*
* @brief   Set Vlan Port MAC Counters configuration for traffic to CPU test.
*/
GT_VOID prvTgfPortMacCountersToCpuVlanConfigurationSet
(
    GT_VOID
)
{
    GT_U32                      portIter;
    GT_STATUS                   rc = GT_OK;
    CPSS_PORTS_BMP_STC          portsTagging;
    CPSS_PORTS_BMP_STC          portsMembers;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);
    }

    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
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
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* AUTODOC: create VLAN 2 with ports [0,1,2,3], unkUCCmd=TRAP_TO_CPU */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 PRV_TGF_VLANID_CNS,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfPortTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
*                                       None
*/
static GT_VOID prvTgfPortTestPacketSend
(
    IN GT_U32           portNum,
    IN TGF_PACKET_STC *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, burstCount, numVfd, vfdArray);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d , %d",
                                 prvTgfDevNum, burstCount, numVfd );

    utfPrintKeepAlive();

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    if(rc == GT_HW_ERROR)
    {
        GT_U32   _portNum;
        GT_U32  portIter;/* port iterator */

        cpssOsTimerWkAfter(20);

        /* give it another try , but first clear all the counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            _portNum = prvTgfPortsArray[portIter];
            PRV_UTF_LOG2_MAC("=======  : dev[%d]port[%d] reset counters  =======\n",
                prvTgfDevNum, _portNum);

            /* reset counters */
            rc = prvTgfResetCountersEth(prvTgfDevNum, _portNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, _portNum);
        }

        /* send Packet from port portNum */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    }
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPortToCpuTrafficFromPortAndDevice function
* @endinternal
*
* @brief   Generate and test traffic from port on device
*
* @param[in] cpuPortEnabled           - GT_TRUE - cpu port enabled - traffic expected
*                                      GT_FALSE - otherwise
*                                       None
*/
static GT_VOID prvTgfPortToCpuTrafficFromPortAndDevice
(
    GT_BOOL cpuPortEnabled
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    CPSS_INTERFACE_INFO_STC portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCntrsCaptured;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: enable PCL ingress policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];

    /* enable capture on port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 1. Sending unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast =======\n");

    /* Set DA MAC Address to be 00:00:00:00:11:11 */
    prvTgfPacketL2Part.daMac[0] = 0x00;
    prvTgfPacketL2Part.daMac[1] = 0x00;
    prvTgfPacketL2Part.daMac[2] = 0x00;
    prvTgfPacketL2Part.daMac[3] = 0x00;
    prvTgfPacketL2Part.daMac[4] = 0x11;
    prvTgfPacketL2Part.daMac[5] = 0x11;

    /* AUTODOC: send UC packet from port 2 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], &prvTgfPacketInfo,1 ,0 ,NULL);
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 - CPU port enabled */
    /* AUTODOC:   No traffic - CPU port disabled */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC:   port 2: ucPktsRcv=1 - CPU port enabled */
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_SEND_PORT_IDX_2_CNS == portIter) && cpuPortEnabled) ?
                                        prvTgfBurstCount : 0, portCntrs.goodPktsRcv.l[0],
                                     "get another goodPktsRcv counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_SEND_PORT_IDX_2_CNS == portIter) && cpuPortEnabled) ?
                                        prvTgfBurstCount : 0, portMacCntrsCaptured.ucPktsRcv.l[0],
                                     "get another captured ucPktsRcv counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_SEND_PORT_IDX_2_CNS == portIter) && cpuPortEnabled) ?
                                        (PRV_TGF_PACKET_CRC_LEN_CNS * prvTgfBurstCount) : 0,
                                        portMacCntrsCaptured.goodOctetsRcv.l[0],
                                     "get another captured goodOctetsRcv counter than expected");
        /* AUTODOC:   port 2: ucPktsSent=1 - CPU port enabled */
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_SEND_PORT_IDX_2_CNS == portIter) && cpuPortEnabled) ?
                                        prvTgfBurstCount : 0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_SEND_PORT_IDX_2_CNS == portIter) && cpuPortEnabled) ?
                                        prvTgfBurstCount : 0, portMacCntrsCaptured.ucPktsSent.l[0],
                                     "get another captured ucPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_SEND_PORT_IDX_2_CNS == portIter) && cpuPortEnabled) ?
                                        (PRV_TGF_PACKET_CRC_LEN_CNS * prvTgfBurstCount) : 0,
                                        portMacCntrsCaptured.goodOctetsSent.l[0],
                                     "get another captured goodOctetsSent counter than expected");
    }
    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);

    if(GT_TRUE == cpuPortEnabled)
    {/* Get entry from captured packet's table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                      GT_TRUE, GT_TRUE, trigPacketBuff,
                                      &buffLen, &packetActualLength,
                                      &prvTgfDevNum, &queue,
                                      &rxParam);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                     TGF_PACKET_TYPE_CAPTURE_E, GT_TRUE);

        /* AUTODOC: verify trapped to CPU captured packet has port 2 */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                     rxParam.portNum,
                                     "sampled packet port num different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfDevNum,
                                     rxParam.hwDevNum,
                                     "sampled packet port num different then expected");
    }

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &prvTgfDevNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_CAPTURE_E, GT_TRUE);

    /* AUTODOC: disable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);
}

/**
* @internal prvTgfPortToCpuTraffic function
* @endinternal
*
* @brief   Generate and test traffic
*
* @param[in] cpuPortEnabled           - GT_TRUE - cpu port enabled - traffic expected
*                                      GT_FALSE - otherwise
*                                       None
*/
GT_VOID prvTgfPortToCpuTraffic
(
    GT_BOOL cpuPortEnabled
)
{
    GT_STATUS   rc = GT_OK;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* send packet to port and device */
    if((CPSS_NET_CPU_PORT_MODE_SDMA_E == PRV_CPSS_PP_MAC(prvTgfDevNum)->cpuPortMode)
#ifdef CHX_FAMILY
        && (!PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfDevNum))
#endif
        )
    {/* cpssDxChPortEnableSet should not have influence while SDMA used */
        prvTgfPortToCpuTrafficFromPortAndDevice(GT_TRUE);
    }
    else
    {
        prvTgfPortToCpuTrafficFromPortAndDevice(cpuPortEnabled);
    }

    /* disable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_FALSE);
}

/**
* @internal prvTgfPortMacCountersConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfPortMacCountersConfigurationSet
(
    GT_VOID
)
{
    GT_U32          portsCount = prvTgfPortsNum;
    GT_U32          portIter   = 0;
    GT_STATUS       rc = GT_OK;

    /* Disable Clear on Read */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* AUTODOC: enable MAC counters clear on read for all ports */
        rc = prvTgfPortMacCountersClearOnReadSet(prvTgfPortsArray[portIter], GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersClearOnReadSet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: clear counters on all ports */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: disable MAC counters clear on read for all ports */
        rc = prvTgfPortMacCountersClearOnReadSet(prvTgfPortsArray[portIter], GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersClearOnReadSet FAILED, rc = [%d]", rc);
        }
    }
}

/**
* @internal prvTgfPortMacCountersRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPortMacCountersRestore
(
    GT_VOID
)
{
    GT_U32          portsCount = prvTgfPortsNum;
    GT_U32          portIter   = 0;
    GT_STATUS       rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: enable updating Rx histogram counters for port 0 */
    rc = prvTgfPortMacCountersRxHistogramEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersRxHistogramEnable FAILED, rc = [%d]", rc);
    }

    /* AUTODOC: enable updating Rx histogram counters for port 2 */
    rc = prvTgfPortMacCountersRxHistogramEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersRxHistogramEnable FAILED, rc = [%d]", rc);
    }

    /* Enable Clear on Read */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* AUTODOC: enable MAC counters clear on read for all ports */
        rc = prvTgfPortMacCountersClearOnReadSet(prvTgfPortsArray[portIter], GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersClearOnReadSet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: clear counters on all ports */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

/**
* @internal prvTgfPortMacCounterPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPortMacCounterPacketSend
(
    IN  GT_U32                        portNum,
    IN  TGF_PACKET_STC               *packetInfoPtr
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portsCount = prvTgfPortsNum;
    GT_U32          portIter   = 0;


    /* Send packet Vlan Tagged */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);
}

/**
* @internal prvTgPortMacCounterCaptureBroadcastTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = ff:ff:ff:ff:ff:ff,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Broadcast Packet sent from port 0.
*         1 Broadcast Packets received on ports 8, 18 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureBroadcastTrafficGenerate
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCntrsCaptured;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);


    /* -------------------------------------------------------------------------
     * 1. Sending broadcast
     */
    PRV_UTF_LOG0_MAC("======= Sending broadcast =======\n");

    /* Set DA MAC Address to be ff:ff:ff:ff:ff:ff */
    prvTgfPacketL2Part.daMac[0] = 0xff;
    prvTgfPacketL2Part.daMac[1] = 0xff;
    prvTgfPacketL2Part.daMac[2] = 0xff;
    prvTgfPacketL2Part.daMac[3] = 0xff;
    prvTgfPacketL2Part.daMac[4] = 0xff;
    prvTgfPacketL2Part.daMac[5] = 0xff;

    /* AUTODOC: send BC packet from port 0 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC:   port 0: brdcPktsRcv=1, ucPktsRcv=mcPktsRcv=0 */
        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.brdcPktsRcv.l[0],
                                         "get another captured brdcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.mcPktsRcv.l[0],
                                         "get another captured mcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS * prvTgfBurstCount), portMacCntrsCaptured.goodOctetsRcv.l[0],
                                         "get another captured goodOctetsRcv counter than expected");
            continue;
        }

        /* AUTODOC:   ports 1,2,3: brdcPktsSent=1, ucPktsSent=mcPktsSent=0 */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.brdcPktsSent.l[0],
                                     "get another captured brdcPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.ucPktsSent.l[0],
                                     "get another captured ucPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.mcPktsSent.l[0],
                                     "get another captured mcPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_PACKET_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount),
                                     portMacCntrsCaptured.goodOctetsSent.l[0],
                                     "get another captured goodOctetsSent counter than expected");
    }
}

/**
* @internal prvTgPortMacCounterCaptureUnicastTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Unicast Packet sent from port 18.
*         1 Unicast Packets received on ports 0, 8 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureUnicastTrafficGenerate
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCntrsCaptured;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);


    /* -------------------------------------------------------------------------
     * 1. Sending unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast =======\n");

    /* Set DA MAC Address to be 00:00:00:00:11:11 */
    prvTgfPacketL2Part.daMac[0] = 0x00;
    prvTgfPacketL2Part.daMac[1] = 0x00;
    prvTgfPacketL2Part.daMac[2] = 0x00;
    prvTgfPacketL2Part.daMac[3] = 0x00;
    prvTgfPacketL2Part.daMac[4] = 0x11;
    prvTgfPacketL2Part.daMac[5] = 0x11;

    /* AUTODOC: send UC packet from port 2 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify traffic on ports 0,1,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC:   port 2: ucPktsRcv=1, brdcPktsRcv=mcPktsRcv=0 */
        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.brdcPktsRcv.l[0],
                                         "get another captured brdcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.mcPktsRcv.l[0],
                                         "get another captured mcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS * prvTgfBurstCount), portMacCntrsCaptured.goodOctetsRcv.l[0],
                                         "get another captured goodOctetsRcv counter than expected");
            continue;
        }

        /* AUTODOC:   ports 0,1,3: ucPktsSent=1, brdcPktsSent=mcPktsSent=0 */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.ucPktsSent.l[0],
                                     "get another captured ucPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.brdcPktsSent.l[0],
                                     "get another captured brdcPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.mcPktsSent.l[0],
                                     "get another captured mcPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_PACKET_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount),
                                     portMacCntrsCaptured.goodOctetsSent.l[0],
                                     "get another captured goodOctetsSent counter than expected");
    }
}

/**
* @internal prvTgPortMacCounterCaptureCpuPortDisable function
* @endinternal
*
* @brief   Disable CPU port and send traffic
*
* @note Only MII CPU port connection disabled
*
*/
GT_VOID prvTgPortMacCounterCaptureCpuPortDisable
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCntrsCaptured;
    GT_U32                          expectedValue, expectedBytesCount, expGoodOctetsRcv;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);


    /* -------------------------------------------------------------------------
     * 1. Sending unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast =======\n");

    /* Set DA MAC Address to be 00:00:00:00:11:11 */
    prvTgfPacketL2Part.daMac[0] = 0x00;
    prvTgfPacketL2Part.daMac[1] = 0x00;
    prvTgfPacketL2Part.daMac[2] = 0x00;
    prvTgfPacketL2Part.daMac[3] = 0x00;
    prvTgfPacketL2Part.daMac[4] = 0x11;
    prvTgfPacketL2Part.daMac[5] = 0x11;

    rc = prvTgfPortEnableSet(prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortEnableSet: %d, %d, GT_FALSE",
                                 prvTgfDevNum, CPSS_CPU_PORT_NUM_CNS);

    /* send packets */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    if((CPSS_NET_CPU_PORT_MODE_SDMA_E == PRV_CPSS_PP_MAC(prvTgfDevNum)->cpuPortMode)
#ifdef CHX_FAMILY
       &&
        (!PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(prvTgfDevNum))
#endif
       )
    {/* cpssDxChPortEnableSet should not have influence while SDMA used */
        expectedValue = 1;
        expectedBytesCount = 64;
        expGoodOctetsRcv = 68;
    }
    else
    {
        expectedValue = 0;
        expectedBytesCount = 0;
        expGoodOctetsRcv = 0;
    }

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* Check Port  */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, portMacCntrsCaptured.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.brdcPktsRcv.l[0],
                                         "get another captured brdcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.mcPktsRcv.l[0],
                                         "get another captured mcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expGoodOctetsRcv, portMacCntrsCaptured.goodOctetsRcv.l[0],
                                         "get another captured goodOctetsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, portMacCntrsCaptured.ucPktsSent.l[0],
                                     "get another captured ucPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.brdcPktsSent.l[0],
                                     "get another captured brdcPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.mcPktsSent.l[0],
                                     "get another captured mcPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedBytesCount,
                                     portMacCntrsCaptured.goodOctetsSent.l[0],
                                     "get another captured goodOctetsSent counter than expected");
    }
}

/**
* @internal prvTgPortMacCounterCaptureMulticastTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = 01:02:03:04:05:06,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Multicast Packet sent from port 0.
*         1 Multicast Packets received on ports 8, 18 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureMulticastTrafficGenerate
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCntrsCaptured;
    GT_U16                          vidx;
    CPSS_PORTS_BMP_STC              portBitmap;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    vidx = 1;

    /* clear entry */
    cpssOsMemSet(&portBitmap, 0, sizeof(portBitmap));

    /* ports 0 and 18 */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);

    /* AUTODOC: create VIDX 1 with ports [0,2] */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &portBitmap);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in prvTgfBrgMcEntryWrite %d", GT_TRUE);

    /* configure FDB entry */
    prvTgfPortMacCountersFdbConfigurationSet();

    /* -------------------------------------------------------------------------
     * 1. Sending multicast
     */
    PRV_UTF_LOG0_MAC("======= Sending multicast =======\n");

    /* Set DA MAC Address to be 01:02:03:04:05:06 */
    prvTgfPacketL2Part.daMac[0] = 0x01;
    prvTgfPacketL2Part.daMac[1] = 0x02;
    prvTgfPacketL2Part.daMac[2] = 0x03;
    prvTgfPacketL2Part.daMac[3] = 0x04;
    prvTgfPacketL2Part.daMac[4] = 0x05;
    prvTgfPacketL2Part.daMac[5] = 0x06;

    /* AUTODOC: send MC packet from port 0 with: */
    /* AUTODOC:   DA=01:02:03:04:05:06, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   DA=01:02:03:04:05:06, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC:   port 0: mcPktsRcv=1, brdcPktsRcv=ucPktsRcv=0 */
        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.mcPktsRcv.l[0],
                                         "get another captured mcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.brdcPktsRcv.l[0],
                                         "get another captured brdcPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_CRC_LEN_CNS * prvTgfBurstCount), portMacCntrsCaptured.goodOctetsRcv.l[0],
                                         "get another captured goodOctetsRcv counter than expected");
            continue;
        }

        /* AUTODOC:   ports 2: mcPktsSent=1, brdcPktsSent=ucPktsSent=0 */
        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.mcPktsSent.l[0],
                                     "get another captured mcPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.brdcPktsSent.l[0],
                                         "get another captured brdcPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.ucPktsSent.l[0],
                                         "get another captured ucPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(((PRV_TGF_PACKET_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount),
                                         portMacCntrsCaptured.goodOctetsSent.l[0],
                                         "get another captured goodOctetsSent counter than expected");
        }
    }

    /* Invalidate MC group */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);

    /* AUTODOC: invalidate MC group */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &portBitmap);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in prvTgfBrgMcEntryWrite %d", GT_TRUE);

    /* Delete FDB entry */
    prvTgfPortMacCountersFdbEntryDelete();
}

/**
* @internal prvTgPortMacCounterCaptureOversizeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0 packets:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB Captured counters:
*         1 Unicast Packet sent from port 0.
*         1 Oversize Packets received on ports 8, 18 and 23.
*/
GT_VOID prvTgPortMacCounterCaptureOversizeTrafficGenerate
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCntrsCaptured;
    GT_U32                          defaultMru;
    GT_U32                          mruSize;

    /* AUTODOC: GENERATE TRAFFIC: */

    rc = prvTgfPortMruGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &defaultMru);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in prvTgfPortMruGet %d", GT_TRUE);

    /* AUTODOC: set Rx MRU to 100 bytes for port 0 */
    mruSize = 100;
    rc = prvTgfPortMruSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], mruSize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in prvTgfPortMruSet %d", GT_TRUE);

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);


    /* -------------------------------------------------------------------------
     * 1. Sending unicast oversize
     *    packet size must be bigger then configured MRU .
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast oversize packet =======\n");

    /* Set PayLoad length to mruSize + 100 bytes */
    prvTgfPacketPayloadPart.dataLength = mruSize + 100;
    prvTgfPacketPayloadPart.dataPtr = cpssOsMalloc(prvTgfPacketPayloadPart.dataLength);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)prvTgfPacketPayloadPart.dataPtr,
                                    "Error in cpssOsMalloc %d", prvTgfPacketPayloadPart.dataLength);
    if (prvTgfPacketPayloadPart.dataPtr != NULL)
    {
        cpssOsMemSet(prvTgfPacketPayloadPart.dataPtr, 0x0, prvTgfPacketPayloadPart.dataLength);
        prvTgfPacketInfo.totalLen =
                TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

        /* Set DA MAC Address to be 00:00:00:00:11:11 */
        prvTgfPacketL2Part.daMac[0] = 0x00;
        prvTgfPacketL2Part.daMac[1] = 0x00;
        prvTgfPacketL2Part.daMac[2] = 0x00;
        prvTgfPacketL2Part.daMac[3] = 0x00;
        prvTgfPacketL2Part.daMac[4] = 0x11;
        prvTgfPacketL2Part.daMac[5] = 0x11;

        /* AUTODOC: send UC packet (216 bytes) from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
        prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
        cpssOsTimerWkAfter(10);

        /* AUTODOC: verify no Tx traffic on ports 1,2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
            }

            /* AUTODOC: verify captured ethernet MAC counters: */
            rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
            }

            /* AUTODOC:   port 0: oversizePkts=1, badOctetsRcv=100 */
            if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.oversizePkts.l[0],
                                             "get another oversizePkts counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(mruSize * prvTgfBurstCount, portMacCntrsCaptured.badOctetsRcv.l[0],
                                             "get another badOctetsRcv counter than expected");
                continue;
            }

                 /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

        cpssOsFree(prvTgfPacketPayloadPart.dataPtr);
    }

    /* set back the packet payload part and size */
    prvTgfPacketPayloadPart.dataLength = sizeof(prvTgfPayloadDataArr);
    prvTgfPacketPayloadPart.dataPtr = prvTgfPayloadDataArr;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;


    /* AUTODOC: restore default MRU for port 0 */
    rc = prvTgfPortMruSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], defaultMru);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in prvTgfPortMruSet %d", GT_TRUE);
}


/**
* @internal prvTgPortMacCounterCaptureHistogramTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0packets:
*         macDa = 01:02:03:04:05:06,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         Histogram Captured counters:
*         Check, that when the feature is disabled, the counters are not updated.
*         Check, that when the feature is enabled, the counters are updated.
*         Check for different packet sizes:
*         packetSize = 64.
*         64<packetSize<128
*         127<packetSize<256
*         255<packetSize<512
*         511<packetSize<1024
*         1023<packetSize
*/
GT_VOID prvTgPortMacCounterCaptureHistogramTrafficGenerate
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCntrsCaptured;
    GT_U32                          burstCount;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* Set DA MAC Address to be 00:00:00:00:11:11 */
    prvTgfPacketL2Part.daMac[0] = 0x00;
    prvTgfPacketL2Part.daMac[1] = 0x00;
    prvTgfPacketL2Part.daMac[2] = 0x00;
    prvTgfPacketL2Part.daMac[3] = 0x00;
    prvTgfPacketL2Part.daMac[4] = 0x11;
    prvTgfPacketL2Part.daMac[5] = 0x11;


    burstCount = prvTgfBurstCount;

    /* ----- Disable both Rx and Tx Histogram counters ----- */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: disable updating Rx histogram counters for all ports */
        rc = prvTgfPortMacCountersRxHistogramEnable(prvTgfPortsArray[portIter], GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersRxHistogramEnable FAILED, rc = [%d]", rc);
        }
        if ((GT_BAD_STATE == rc) &&
            prvCpssDxChPortRemotePortCheck(prvTgfDevNum,prvTgfPortsArray[portIter]))
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersRxHistogramEnable disable Not Supported on remote port, rc = [%d]\n", rc);
            burstCount = 0;
        }

        /* AUTODOC: disable updating Tx histogram counters for all ports */
        rc = prvTgfPortMacCountersTxHistogramEnable(prvTgfPortsArray[portIter], GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersTxHistogramEnable FAILED, rc = [%d]", rc);
        }
        if ((GT_BAD_STATE == rc) &&
            prvCpssDxChPortRemotePortCheck(prvTgfDevNum,prvTgfPortsArray[portIter]))
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersRxHistogramEnable disable Not Supported on remote port, rc = [%d]\n", rc);
            burstCount = 0;
        }
    }

    if (burstCount > 0)
    {

        /* -------------------------------------------------------------------------
         * 1. Sending unicast packet, Rx/Tx Histogram disabled
         *    Packet size is 64 bytes
         */
        PRV_UTF_LOG0_MAC("======= Sending unicast packet (packetSize = 64) =======\n");
        PRV_UTF_LOG0_MAC("======= Rx/Tx Histogram disabled =======\n");

        /* AUTODOC: send UC packet (64 bytes) from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
        prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
        cpssOsTimerWkAfter(10);


        /* AUTODOC: verify traffic on ports 1,2,3: */
        /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
            }

            /* AUTODOC: verify captured ethernet MAC counters: */
            rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
            }

            /* AUTODOC:   port 0: all Rx Histogram counters 0 */
            if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                             "get another captured pkts64Octets counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                             "get another captured pkts65to127Octets counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                             "get another captured pkts128to255Octets counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                             "get another captured pkts256to511Octets counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                             "get another captured pkts512to1023Octets counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                             "get another captured pkts1024tomaxOoctets counter than expected");
                continue;
            }

            /* AUTODOC:   port 1,2,3: all Tx Histogram counters 0 */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
        }
    }

    /* Pay attention: as the counters are not cleared on read,
       their values are accumulated */


    /* ------------- Enable Rx Histogram --------------- */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: enable updating Rx histogram counters for all ports */
        rc = prvTgfPortMacCountersRxHistogramEnable(prvTgfPortsArray[portIter], GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersRxHistogramEnable FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: disable updating Tx histogram counters for all ports */
        rc = prvTgfPortMacCountersTxHistogramEnable(prvTgfPortsArray[portIter], GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersTxHistogramEnable FAILED, rc = [%d]", rc);
        }
    }

    /* -------------------------------------------------------------------------
     * 2. Sending unicast packet, Rx Histogram enabled and Tx Histogram disabled
     *    Packet size is 64 bytes
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packet (packetSize = 64) =======\n");
    PRV_UTF_LOG0_MAC("==== Rx Histogram enabled and Tx Histogram disabled ====\n");

    /* AUTODOC: send UC packet (64 bytes) from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);


    burstCount += prvTgfBurstCount;

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC:   port 0: Rx pkts65to127Octets=1 */
        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
            continue;
        }

        /* AUTODOC:   port 1,2,3: all Tx Histogram counters 0 */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                     "get another captured pkts64Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                     "get another captured pkts65to127Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                     "get another captured pkts128to255Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                     "get another captured pkts256to511Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                     "get another captured pkts512to1023Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                     "get another captured pkts1024tomaxOoctets counter than expected");
    }


    /* ---------- Enable Tx Histogram ---------- */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: enable updating Tx histogram counters for all ports */
        rc = prvTgfPortMacCountersTxHistogramEnable(prvTgfPortsArray[portIter], GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersRxHistogramEnable FAILED, rc = [%d]", rc);
        }
    }

    /* -------------------------------------------------------------------------
     * 3. Sending unicast packet, Rx/Tx Histogram enabled
     *    Packet size is 64 bytes
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packet (packetSize = 64) =======\n");
    PRV_UTF_LOG0_MAC("======= Rx/Tx Histogram enabled  ========\n");

    /* AUTODOC: send UC packet (64 bytes) from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount += prvTgfBurstCount;

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC:   port 0: Rx pkts65to127Octets=1 */
        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 3), portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
            continue;
        }

        /* AUTODOC:   port 1,2,3: Tx pkts64Octets=1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts64Octets.l[0],
                                     "get another captured pkts64Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                     "get another captured pkts65to127Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                     "get another captured pkts128to255Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                     "get another captured pkts256to511Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                     "get another captured pkts512to1023Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                     "get another captured pkts1024tomaxOoctets counter than expected");
    }


    /* -------------------------------------------------------------------------
     * 4. Sending unicast packets with  64<packetSize<128
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packet 64<packetSize<128 =======\n");

    /* Set PayLoad length to 80 bytes */
    cpssOsMemSet(prvTgfPayloadData80BytesArr, 0x0, sizeof(prvTgfPayloadData80BytesArr));
    prvTgfPacketPayloadPart.dataLength = sizeof(prvTgfPayloadData80BytesArr);
    prvTgfPacketPayloadPart.dataPtr = prvTgfPayloadData80BytesArr;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* AUTODOC: send UC packet (96 bytes) from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount += prvTgfBurstCount;

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* AUTODOC:   port 0: Rx pkts65to127Octets=5 */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 5), portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
            continue;
        }

        /* AUTODOC:   port 1,2,3: Tx pkts64Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts65to127Octets=1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                     "get another captured pkts65to127Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts64Octets.l[0],
                                     "get another captured pkts64Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                     "get another captured pkts128to255Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                     "get another captured pkts256to511Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                     "get another captured pkts512to1023Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                     "get another captured pkts1024tomaxOoctets counter than expected");
    }

    /* -------------------------------------------------------------------------
     * 5. Sending unicast packets with  127<packetSize<256
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packet 127<packetSize<256 =======\n");

    /* Set PayLoad length to 200 bytes */
    cpssOsMemSet(prvTgfPayloadData200BytesArr, 0x0, sizeof(prvTgfPayloadData200BytesArr));
    prvTgfPacketPayloadPart.dataLength = sizeof(prvTgfPayloadData200BytesArr);
    prvTgfPacketPayloadPart.dataPtr = prvTgfPayloadData200BytesArr;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* AUTODOC: send UC packet (216 bytes) from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount += prvTgfBurstCount;

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* AUTODOC:   port 0: Rx pkts65to127Octets=5 */
            /* AUTODOC:   port 0: Rx pkts128to255Octets=2 */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 5), portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
            continue;
        }

        /* AUTODOC:   port 1,2,3: Tx pkts64Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts65to127Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts128to255Octets=1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                     "get another captured pkts128to255Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts64Octets.l[0],
                                     "get another captured pkts64Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                     "get another captured pkts65to127Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                     "get another captured pkts256to511Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                     "get another captured pkts512to1023Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                     "get another captured pkts1024tomaxOoctets counter than expected");
    }

    /* -------------------------------------------------------------------------
     * 6. Sending unicast packets with  255<packetSize<512
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packet 255<packetSize<512 =======\n");

    /* Set PayLoad length to 300 bytes */
    cpssOsMemSet(prvTgfPayloadData300BytesArr, 0x0, sizeof(prvTgfPayloadData300BytesArr));
    prvTgfPacketPayloadPart.dataLength = sizeof(prvTgfPayloadData300BytesArr);
    prvTgfPacketPayloadPart.dataPtr = prvTgfPayloadData300BytesArr;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* AUTODOC: send UC packet (316 bytes) from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount += prvTgfBurstCount;

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* AUTODOC:   port 0: Rx pkts65to127Octets=5 */
            /* AUTODOC:   port 0: Rx pkts128to255Octets=2 */
            /* AUTODOC:   port 0: Rx pkts256to511Octets=2 */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 5), portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
            continue;
        }

        /* AUTODOC:   port 1,2,3: Tx pkts64Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts65to127Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts128to255Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts256to511Octets=1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                     "get another captured pkts256to511Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts64Octets.l[0],
                                     "get another captured pkts64Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                     "get another captured pkts65to127Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                     "get another captured pkts128to255Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                     "get another captured pkts512to1023Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                     "get another captured pkts1024tomaxOoctets counter than expected");
    }

    /* -------------------------------------------------------------------------
     * 7. Sending unicast packets with  511<packetSize<1024
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packet 511<packetSize<1024 =======\n");

    /* Set PayLoad length to 300 bytes */
    cpssOsMemSet(prvTgfPayloadData500BytesArr, 0x0, sizeof(prvTgfPayloadData500BytesArr));
    prvTgfPacketPayloadPart.dataLength = sizeof(prvTgfPayloadData500BytesArr);
    prvTgfPacketPayloadPart.dataPtr = prvTgfPayloadData500BytesArr;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* AUTODOC: send UC packet (516 bytes) from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount += prvTgfBurstCount;

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* AUTODOC:   port 0: Rx pkts65to127Octets=5 */
            /* AUTODOC:   port 0: Rx pkts128to255Octets=2 */
            /* AUTODOC:   port 0: Rx pkts256to511Octets=2 */
            /* AUTODOC:   port 0: Rx pkts512to1023Octets=2 */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2),
                                         portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 5), portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
            continue;
        }

        /* AUTODOC:   port 1,2,3: Tx pkts64Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts65to127Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts128to255Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts256to511Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts512to1023Octets=1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                     "get another captured pkts512to1023Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts64Octets.l[0],
                                     "get another captured pkts64Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                     "get another captured pkts65to127Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                     "get another captured pkts128to255Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                     "get another captured pkts256to511Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                     "get another captured pkts1024tomaxOoctets counter than expected");
    }


    /* -------------------------------------------------------------------------
     * 8. Sending unicast packets with  1023<packetSize
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast packet 1023<packetSize  =======\n");

    /* Set PayLoad length to 300 bytes */
    cpssOsMemSet(prvTgfPayloadData1024BytesArr, 0x0, sizeof(prvTgfPayloadData1024BytesArr));
    prvTgfPacketPayloadPart.dataLength = sizeof(prvTgfPayloadData1024BytesArr);
    prvTgfPacketPayloadPart.dataPtr = prvTgfPayloadData1024BytesArr;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* AUTODOC: send UC packet (1040 bytes) from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_0_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount += prvTgfBurstCount;

    /* AUTODOC: verify traffic on ports 1,2,3: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify captured ethernet MAC counters: */
        rc = prvTgfPortMacCountersCaptureOnPortGet(prvTgfPortsArray[portIter], &portMacCntrsCaptured);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersCaptureOnPortGet FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_0_CNS == portIter)
        {
            /* AUTODOC:   port 0: Rx pkts65to127Octets=5 */
            /* AUTODOC:   port 0: Rx pkts128to255Octets=2 */
            /* AUTODOC:   port 0: Rx pkts256to511Octets=2 */
            /* AUTODOC:   port 0: Rx pkts512to1023Octets=2 */
            /* AUTODOC:   port 0: Rx pkts1024tomaxOoctets=2 */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2),
                                         portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                         "get another captured pkts1024tomaxOoctets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portMacCntrsCaptured.pkts64Octets.l[0],
                                         "get another captured pkts64Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 5), portMacCntrsCaptured.pkts65to127Octets.l[0],
                                         "get another captured pkts65to127Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts128to255Octets.l[0],
                                         "get another captured pkts128to255Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts256to511Octets.l[0],
                                         "get another captured pkts256to511Octets counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * 2), portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                         "get another captured pkts512to1023Octets counter than expected");
            continue;
        }

        /* AUTODOC:   port 1,2,3: Tx pkts64Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts65to127Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts128to255Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts256to511Octets=1 */
        /* AUTODOC:   port 1,2,3: Tx pkts1024tomaxOoctets=1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts1024tomaxOoctets.l[0],
                                     "get another captured pkts1024tomaxOoctets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts64Octets.l[0],
                                     "get another captured pkts64Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts65to127Octets.l[0],
                                     "get another captured pkts65to127Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts128to255Octets.l[0],
                                     "get another captured pkts128to255Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts256to511Octets.l[0],
                                     "get another captured pkts256to511Octets counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portMacCntrsCaptured.pkts512to1023Octets.l[0],
                                     "get another captured pkts512to1023Octets counter than expected");


        /* return the packet data and size to previous one */
        prvTgfPacketPayloadPart.dataLength = sizeof(prvTgfPayloadDataArr);
        prvTgfPacketPayloadPart.dataPtr = prvTgfPayloadDataArr;
        prvTgfPacketInfo.totalLen =
                TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;
    }
}


/**
* @internal prvTgPortMacCounterClearOnReadTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send from port 0 packets:
*         macDa = 00:00:00:00:11:11,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         MAC MIB counters:
*         Check that when the feature enabled, counters are cleared on read.
*         Check that when the feature disabled, counters are not cleared on read.
*/
GT_VOID prvTgPortMacCounterClearOnReadTrafficGenerate
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          burstCount;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* -------------------------------------------------------------------------
     * 1. Sending unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast =======\n");

    /* Set DA MAC Address to be 00:00:00:00:11:11 */
    prvTgfPacketL2Part.daMac[0] = 0x00;
    prvTgfPacketL2Part.daMac[1] = 0x00;
    prvTgfPacketL2Part.daMac[2] = 0x00;
    prvTgfPacketL2Part.daMac[3] = 0x00;
    prvTgfPacketL2Part.daMac[4] = 0x11;
    prvTgfPacketL2Part.daMac[5] = 0x11;

    /* AUTODOC: send UC packet from port 2 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount = prvTgfBurstCount;

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: disable MAC counters clear on read for all ports */
        rc = prvTgfPortMacCountersClearOnReadSet(prvTgfPortsArray[portIter], GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersClearOnReadSet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: read counters on all ports: */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* AUTODOC:   port 2: goodPktsRcv=ucPktsRcv=1 */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
        }
        else
        {
            /* AUTODOC:   port 0,1,3: goodPktsSent=ucPktsSent=1 */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                         "get another captured ucPktsSent counter than expected");
        }


        /* AUTODOC: read counters again - verify that they were not cleared */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
        }
        else
        {
             /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                         "get another captured ucPktsSent counter than expected");
        }
    }

    /* AUTODOC: send UC packet from port 2 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortMacCounterPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    burstCount += prvTgfBurstCount;

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* Enable Clear on Read */
        rc = prvTgfPortMacCountersClearOnReadSet(prvTgfPortsArray[portIter], GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPortMacCountersClearOnReadSet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: read counters on all ports: */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* AUTODOC:   port 2: goodPktsRcv=ucPktsRcv=2 */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
        }
        else
        {
            /* AUTODOC:   port 0,1,3: goodPktsSent=ucPktsSent=1 */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.ucPktsSent.l[0],
                                         "get another captured ucPktsSent counter than expected");
        }

        /* AUTODOC: read counters again - verify that they were cleared */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0],
                                         "get another captured ucPktsRcv counter than expected");
        }
        else
        {
             /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                         "get another captured ucPktsSent counter than expected");
        }
    }
}

