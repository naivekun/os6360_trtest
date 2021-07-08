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
* @file prvTgfVlanKeepVlan1.c
*
* @brief Bridge Keep Vlan1 UT.
*
* @version   11
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <bridge/prvTgfVlanKeepVlan1.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_2_CNS  2

/* Zero VLAN Id */
#define PRV_TGF_VLANID_ZERO_CNS  0

/* VLAN Id 5*/
#define PRV_TGF_VLANID_5_CNS  5

/* VLAN Id 7*/
#define PRV_TGF_VLANID_7_CNS  7

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to send traffic to */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* Vlan0 up value to send packet with */
#define PRV_TGF_UP_4_CNS     4

/* Vlan0 up value to send packet with */
#define PRV_TGF_UP_6_CNS     6

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x22, 0x22, 0x22, 0x22},               /* dstMac */
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x11}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                            /* pri, cfi, VlanId */
};

/* Second VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_7_CNS                          /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
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

/************************ Single tagged packet ********************************/

/* PARTS of single tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketSingleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of single tagged packet with CRC */
#define PRV_TGF_PACKET_SINGLE_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoSingleTag =
{
    PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketSingleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketSingleTagPartArray                                        /* partsArray */
};

/************************ Untagged packet ********************************/

/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfPacketUntagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of untagged packet */
#define PRV_TGF_PACKET_UNTAG_LEN_CNS TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of untagged packet with CRC */
#define PRV_TGF_PACKET_UNTAG_CRC_LEN_CNS  PRV_TGF_PACKET_UNTAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Untagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoUntag =
{
    PRV_TGF_PACKET_UNTAG_LEN_CNS,                                /* totalLen */
    sizeof(prvTgfPacketUntagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketUntagPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/**
* @enum PRV_TGF_PKT_TAG_TYPE_ENT
 *
 * @brief Enumeration of Packet Tag type.
*/
typedef enum{

    /** packet is untagged */
    PRV_TGF_PKT_TAG_TYPE_UNTAGGED_E,

    /** packet is single tagged */
    PRV_TGF_PKT_TAG_TYPE_SINGLE_TAGGED_E,

    /** packet is double tagged */
    PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E

} PRV_TGF_PKT_TAG_TYPE_ENT;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfVlanKeepVlan1VlanCfg function
* @endinternal
*
* @brief   Set VLAN entry with recieved tag command.
*
* @param[in] vlanId                   -  to be configured
* @param[in] tagCmd                   - VLAN tag command
*                                       None
*/
static GT_VOID prvTgfVlanKeepVlan1VlanCfg
(
    IN GT_U16                            vlanId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT tagCmd
)
{
    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
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
    vlanInfo.stgId                = 0;
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
* @internal prvTgfVlanKeepVlan1TestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfVlanKeepVlan1TestPacketSend
(
    IN TGF_PACKET_STC          *packetInfoPtr
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portsCount = prvTgfPortsNum;
    GT_U32          portIter   = 0;


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


    /* AUTODOC: send configured packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}


/**
* @internal prvTgfVlanKeepVlan1TxEthTriggerCheck function
* @endinternal
*
* @brief   Check Tx Triggers
*
* @param[in] vfdArray[]               - VFD array with expected results
* @param[in] vfdNum                   - VFD number
*                                       None
*/
static GT_VOID prvTgfVlanKeepVlan1TxEthTriggerCheck
(
    IN GT_U8                    vfdNum,
    IN TGF_VFD_INFO_STC         vfdArray[]
)
{
    GT_STATUS                       rc;
    GT_U8                           queue    = 0;
    GT_BOOL                         getFirst = GT_TRUE;
    GT_U32                          buffLen  = 0x600;
    GT_U32                          packetActualLength = 0;
    static  GT_U8                   packetBuff[0x600] = {0};
    TGF_NET_DSA_STC                 rxParam;
    GT_BOOL                         triggered;
    GT_U8                           devNum;


    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);


    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficGeneratorRxInCpuGet %d", TGF_PACKET_TYPE_CAPTURE_E);

    /* Check that the pattern is equal to vfdArray */
    rc = tgfTrafficGeneratorTxEthTriggerCheck(packetBuff,
                                              buffLen,
                                              vfdNum,
                                              vfdArray,
                                              &triggered);
    UTF_VERIFY_EQUAL2_STRING_MAC(rc, GT_OK, "Error in tgfTrafficGeneratorTxEthTriggerCheck %d, %d", buffLen, vfdNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, triggered,
                                 "Packet VLAN TAG0/1 pattern is wrong");
}


/**
* @internal prvTgfVlanKeepVlan1TrafficGenerate function
* @endinternal
*
* @param[in] keepVlan1En              - GT_TRUE - Keep Vlan1 is enabled
*                                      GT_FALSE - Keep Vlan1 is disabled
* @param[in] destPort                 - destination port number
* @param[in] up                       - User priority of Vlan 0
* @param[in] ingressPktTagType        - Ingress tag packet type: untagged, single or
*                                      double
* @param[in] tagCmd                   - tag command
* @param[in] ingrVlan0                - Vlan Tag0 value
* @param[in] ingrVlan1                - Vlan Tag1 value
*                                       None
*/
static GT_VOID prvTgfVlanKeepVlan1TrafficGenerate
(
    IN GT_BOOL                              keepVlan1En,
    IN GT_U32                                destPort,
    IN GT_U8                                up,
    IN PRV_TGF_PKT_TAG_TYPE_ENT             ingressPktTagType,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT    tagCmd,
    IN GT_U16                               ingrVlan0,
    IN GT_U16                               ingrVlan1
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    TGF_VFD_INFO_STC                vfdArray[2];
    GT_U8                           vfdNum;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_BOOL                         vlan1IsKept;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* Configure vlan with Vlan command */
    prvTgfVlanKeepVlan1VlanCfg(ingrVlan0, tagCmd);

    /* Enable/Disable Vlan Tag1 removal */
    rc = prvTgfBrgVlanKeepVlan1EnableSet(prvTgfDevNum, destPort, up, keepVlan1En);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfBrgVlanKeepVlan1EnableSet %d", GT_TRUE);


    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = destPort;

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);

    /* AUTODOC: GENERATE TRAFFIC: */

    /* -------------------------------------------------------------------------
     * 1. Sending unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unicast =======\n");

    /* send packets */
    switch (ingressPktTagType)
    {
        case PRV_TGF_PKT_TAG_TYPE_UNTAGGED_E:
            /* configure default UP on all ports to given up */

            prvTgfVlanKeepVlan1TestPacketSend(&prvTgfPacketInfoUntag);
            break;
        case PRV_TGF_PKT_TAG_TYPE_SINGLE_TAGGED_E:
            prvTgfPacketVlanTag0Part.vid = ingrVlan0;
            prvTgfPacketVlanTag0Part.pri = up;
            prvTgfVlanKeepVlan1TestPacketSend(&prvTgfPacketInfoSingleTag);
            break;
        case PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E:
            prvTgfPacketVlanTag0Part.vid = ingrVlan0;
            prvTgfPacketVlanTag1Part.vid = ingrVlan1;
            prvTgfPacketVlanTag0Part.pri = up;
            prvTgfVlanKeepVlan1TestPacketSend(&prvTgfPacketInfoDoubleTag);
            break;
        default:
            PRV_UTF_LOG1_MAC("Ingress Packet Type is wrong ingressPktTagType = %d", ingressPktTagType);
    }
    cpssOsTimerWkAfter(10);


    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

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

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        if (destPort == prvTgfPortsArray[portIter])
        {
            /* check that packet length includes double tag */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");

            /* Check if Tag1 is kept */
            vlan1IsKept = GT_FALSE;

            /* check if feature enabled */
            if (keepVlan1En == GT_TRUE)
            {
                /* Check if Vlan Tag1 was in ingress packet */
                if (ingressPktTagType == PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E)
                {
                    vlan1IsKept = GT_TRUE;
                }
            }
            else
            {
                /* Vlan Tag1 is according to tag command */

                if ((tagCmd == PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E) ||
                    (tagCmd == PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E) ||
                    (tagCmd == PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E))
                {
                    if (ingressPktTagType == PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E)
                    {
                        vlan1IsKept = GT_TRUE;
                    }
                }
            }

            /* Configure vfdArray */
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
            vfdArray[0].modeExtraInfo = 0;
            vfdArray[0].offset = 12;

            if (vlan1IsKept == GT_FALSE)
            {
                if (tagCmd == PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E)
                {
                    /* packet egresses untagged */
                    vfdArray[0].cycleCount = 4;
                    vfdArray[0].patternPtr[0] = 0x55;
                    vfdArray[0].patternPtr[1] = 0x55;
                    vfdArray[0].patternPtr[2] = 0x00;
                    vfdArray[0].patternPtr[3] = 0x00;
                }
                else
                {
                    /* packet don't have Vlan1 */
                    vfdArray[0].cycleCount = 4;
                    vfdArray[0].patternPtr[0] = 0x81;
                    vfdArray[0].patternPtr[1] = 0x00;
                    vfdArray[0].patternPtr[2] = (GT_U8)(up << 5);
                    vfdArray[0].patternPtr[3] = (GT_U8)ingrVlan0;
                }

                vfdNum = 1;
            }
            else
            {
                if ((tagCmd == PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E) ||
                    (tagCmd == PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E))
                {
                    /* packet has only Vlan1 */
                    vfdArray[0].cycleCount = 4;
                    vfdArray[0].patternPtr[0] = 0x81;
                    vfdArray[0].patternPtr[1] = 0x00;
                    vfdArray[0].patternPtr[2] = 0x00;
                    vfdArray[0].patternPtr[3] = (GT_U8)ingrVlan1;

                    vfdNum = 1;
                }
                else
                {
                    /* packet has Vlan0 and Vlan1 */
                    vfdNum = 2;

                    if (tagCmd == PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E)
                    {
                        /* { Tag1, Tag0 } */
                        vfdArray[0].cycleCount = 4;
                        vfdArray[0].patternPtr[0] = 0x81;
                        vfdArray[0].patternPtr[1] = 0x00;
                        vfdArray[0].patternPtr[2] = 0x00;
                        vfdArray[0].patternPtr[3] = (GT_U8)ingrVlan1;

                        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
                        vfdArray[1].modeExtraInfo = 0;
                        vfdArray[1].offset = 16;
                        vfdArray[1].cycleCount = 4;
                        vfdArray[1].patternPtr[0] = 0x81;
                        vfdArray[1].patternPtr[1] = 0x00;
                        vfdArray[1].patternPtr[2] = (GT_U8)(up << 5);
                        vfdArray[1].patternPtr[3] = (GT_U8)ingrVlan0;
                    }
                    else
                    {
                        /* {Tag0, Tag1 }*/
                        vfdArray[0].cycleCount = 4;
                        vfdArray[0].patternPtr[0] = 0x81;
                        vfdArray[0].patternPtr[1] = 0x00;
                        vfdArray[0].patternPtr[2] = (GT_U8)(up << 5);
                        vfdArray[0].patternPtr[3] = (GT_U8)ingrVlan0;

                        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
                        vfdArray[1].modeExtraInfo = 0;
                        vfdArray[1].offset = 16;
                        vfdArray[1].cycleCount = 4;
                        vfdArray[1].patternPtr[0] = 0x81;
                        vfdArray[1].patternPtr[1] = 0x00;
                        vfdArray[1].patternPtr[2] = 0x00;
                        vfdArray[1].patternPtr[3] = (GT_U8)ingrVlan1;
                    }

                }
            }

            prvTgfVlanKeepVlan1TxEthTriggerCheck(vfdNum, vfdArray);
        }
    }
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfVlanKeepVlan1ConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Generate Traffic:
*         Success Criteria:
*         Additional Configuration:
*         Generate Traffic:
*         Success Criteria:
*/
GT_VOID prvTgfVlanKeepVlan1ConfigurationSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    return;
}


/**
* @internal prvTgfVlanKeepVlan1EnTagStateVlan1Vlan0DoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to inner Tag1 and outer Tag0
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is not removed and it's value 7
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateVlan1Vlan0DoubleTagTrafficGenerate
(
    void
)
{
    /* AUTODOC: create VLAN 2 with "Outer TAG0 Inner Tag1" cmd */
    /* AUTODOC: enable Keep Vlan1 for dest port 2 and up 4 */
    /* AUTODOC: configure to send double tagged packet: */
    /* AUTODOC:   DA = 00:00:22:22:22:22, SA = 00:00:11:11:11:11 */
    /* AUTODOC:   TAG0 = 2, TAG1 = 7 */
    prvTgfVlanKeepVlan1TrafficGenerate(
                    GT_TRUE,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],       /* destination Port */
                    PRV_TGF_UP_4_CNS,                   /* up */
                    PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E,
                    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E, /* tagCmd */
                    PRV_TGF_VLANID_2_CNS,               /* vlan0*/
                    PRV_TGF_VLANID_7_CNS                /* vlan1 */
                    );
    /* AUTODOC: verify on port 2 TAG1 is not removed and it's value 7 */
}


/**
* @internal prvTgfVlanKeepVlan1EnTagStateVlan0DoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 6.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is not removed and it's value 7
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateVlan0DoubleTagTrafficGenerate
(
    void
)
{
    /* AUTODOC: create VLAN 2 with "TAG0" cmd */
    /* AUTODOC: enable Keep Vlan1 for dest port 2 and up 6 */
    /* AUTODOC: configure to send double tagged packet: */
    /* AUTODOC:   DA = 00:00:22:22:22:22, SA = 00:00:11:11:11:11 */
    /* AUTODOC:   TAG0 = 2, TAG1 = 7 */
    prvTgfVlanKeepVlan1TrafficGenerate(
                    GT_TRUE,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],       /* destination Port */
                    PRV_TGF_UP_6_CNS,                   /* up */
                    PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E,
                    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E, /* tagCmd */
                    PRV_TGF_VLANID_2_CNS,               /* vlan0*/
                    PRV_TGF_VLANID_7_CNS                /* vlan1 */
                    );
    /* AUTODOC: verify on port 2 TAG1 is not removed and it's value 7 */
}


/**
* @internal prvTgfVlanKeepVlan1EnTagStateUntaggedDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is not removed and it's value 7
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateUntaggedDoubleTagTrafficGenerate
(
    void
)
{
    /* AUTODOC: create VLAN 2 with "Untagged" cmd */
    /* AUTODOC: enable Keep Vlan1 for dest portIdx 2 and up 4 */
    /* AUTODOC: configure to send double tagged packet: */
    /* AUTODOC:   DA = 00:00:22:22:22:22, SA = 00:00:11:11:11:11 */
    /* AUTODOC:   TAG0 = 2, TAG1 = 7 */
    prvTgfVlanKeepVlan1TrafficGenerate(
                    GT_TRUE,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],       /* destination Port */
                    PRV_TGF_UP_4_CNS,                   /* up */
                    PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E,
                    PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E, /* tagCmd */
                    PRV_TGF_VLANID_2_CNS,               /* vlan0*/
                    PRV_TGF_VLANID_7_CNS                /* vlan1 */
                    );
    /* AUTODOC: verify on port 2 TAG1 is not removed and it's value 7 */
}


/**
* @internal prvTgfVlanKeepVlan1EnTagStateVlan0SingleTagTrafficGenerate function
* @endinternal
*
* @brief   Enable Keep Vlan1 for destination port 18 and up 6.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 single tagged packet, where Tag0 = 2:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanKeepVlan1EnTagStateVlan0SingleTagTrafficGenerate
(
    void
)
{
    /* AUTODOC: create VLAN 2 with "TAG0" cmd */
    /* AUTODOC: enable Keep Vlan1 for dest portIdx 2 and up 6 */
    /* AUTODOC: configure to send single tagged packet: */
    /* AUTODOC:   DA = 00:00:22:22:22:22, SA = 00:00:11:11:11:11 */
    /* AUTODOC:   TAG0 = 2 */
    prvTgfVlanKeepVlan1TrafficGenerate(
                    GT_TRUE,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],       /* destination Port */
                    PRV_TGF_UP_6_CNS,                   /* up */
                    PRV_TGF_PKT_TAG_TYPE_SINGLE_TAGGED_E,
                    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E, /* tagCmd */
                    PRV_TGF_VLANID_2_CNS,               /* vlan0*/
                    PRV_TGF_VLANID_7_CNS                /* vlan1 */
                    );
    /* AUTODOC: verify on port 2 TAG1 is removed */
}


/**
* @internal prvTgfVlanKeepVlan1DisTagStateVlan0DoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Disable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to inner Tag0
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanKeepVlan1DisTagStateVlan0DoubleTagTrafficGenerate
(
    void
)
{
    /* AUTODOC: create VLAN 2 with "TAG0" cmd */
    /* AUTODOC: disable Keep Vlan1 for dest portIdx 2 and up 4 */
    /* AUTODOC: configure to send double tagged packet: */
    /* AUTODOC:   DA = 00:00:22:22:22:22, SA = 00:00:11:11:11:11 */
    /* AUTODOC:   TAG0 = 2, TAG1 = 7 */
    prvTgfVlanKeepVlan1TrafficGenerate(
                    GT_FALSE,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],       /* destination Port */
                    PRV_TGF_UP_4_CNS,                   /* up */
                    PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E,
                    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E, /* tagCmd */
                    PRV_TGF_VLANID_2_CNS,               /* vlan0*/
                    PRV_TGF_VLANID_7_CNS                /* vlan1 */
                    );
    /* AUTODOC: verify on port 2 TAG1 is removed */
}


/**
* @internal prvTgfVlanKeepVlan1DisTagStateUntaggedDoubleTagTrafficGenerate function
* @endinternal
*
* @brief   Disable Keep Vlan1 for destination port 18 and up 4.
*         Configure Vlan command to untagged
*         Generate traffic:
*         Send from port0 double tagged packet, where Tag0 = 2 and Tag1 = 7:
*         macDa = 00:00:22:22:22:22 (port 18)
*         macSa = 00:00:11:11:11:11 (port 0)
*         Success Criteria:
*         Tag1 is removed
*/
GT_VOID prvTgfVlanKeepVlan1DisTagStateUntaggedDoubleTagTrafficGenerate
(
    void
)
{
    /* AUTODOC: create VLAN 2 with "Untagged" cmd */
    /* AUTODOC: disable Keep Vlan1 for dest portIdx 2 and up 4 */
    /* AUTODOC: configure to send double tagged packet: */
    /* AUTODOC:   DA = 00:00:22:22:22:22, SA = 00:00:11:11:11:11 */
    /* AUTODOC:   TAG0 = 2, TAG1 = 7 */
    prvTgfVlanKeepVlan1TrafficGenerate(
                    GT_FALSE,
                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],       /* destination Port */
                    PRV_TGF_UP_4_CNS,                   /* up */
                    PRV_TGF_PKT_TAG_TYPE_DOUBLE_TAGGED_E,
                    PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E, /* tagCmd */
                    PRV_TGF_VLANID_2_CNS,               /* vlan0*/
                    PRV_TGF_VLANID_7_CNS                /* vlan1 */
                    );
    /* AUTODOC: verify on port 2 TAG1 is removed */
}


/**
* @internal prvTgfVlanKeepVlan1ConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfVlanKeepVlan1ConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS       rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: invalidate VLAN entry 2 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: disable Keep Vlan1 */
    rc = prvTgfBrgVlanKeepVlan1EnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_UP_4_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfBrgVlanKeepVlan1EnableSet %d", GT_TRUE);

    rc = prvTgfBrgVlanKeepVlan1EnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], PRV_TGF_UP_6_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfBrgVlanKeepVlan1EnableSet %d", GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}



