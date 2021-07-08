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
* @file prvTgfVlanIngressFiltering.c
*
* @brief VLAN Ingress Filtering Tagged Packets
*
* @version   19
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
#include <bridge/prvTgfVlanIngressFiltering.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS        2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0

/* default VLAN ID */
#define PRV_TGF_DEF_VLAN_ID_CNS     2

/* invalid VLAN ID */
#define PRV_TGF_INV_VLAN_ID_CNS     3

/*  Inlif number */
#define PRV_TGF_INLIF_CNS     6000


/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           2

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 4;


/* VLAN Id array */
static GT_U8        prvTgfVlanIdArray[] = {PRV_TGF_DEF_VLAN_ID_CNS, PRV_TGF_INV_VLAN_ID_CNS};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {{0x00, 0x00, 0x00, 0x00, 0x34, 0x03},
                                               {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}};

/* VLAN tag part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                                          0, 0, PRV_TGF_DEF_VLAN_ID_CNS};

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
                                                      {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
                                                      {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}};









/* GRE array */
static GT_U8 prvTgfPacketGreDataArr[] = {0x00, 0x00, 0x65, 0x58};

/* GRE part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGrePart =
{
    sizeof(prvTgfPacketGreDataArr),  /* dataLength */
    prvTgfPacketGreDataArr           /* dataPtr */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x58,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x2f,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartInt =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x25}     /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPartInt =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_DEF_VLAN_ID_CNS           /* pri, cfi, VlanId */
};


/*********************************** packet to send *******************************/
/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIpv4PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGrePart},
    /* incapsulation ethernet frame part */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartInt},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPartInt},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS \
      TGF_L2_HEADER_SIZE_CNS * 2  + \
      TGF_VLAN_TAG_SIZE_CNS  * 2  + \
      sizeof(prvTgfPacketGreDataArr) + \
      TGF_ETHERTYPE_SIZE_CNS      + \
      TGF_IPV4_HEADER_SIZE_CNS    + \
      sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfIpv4PacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfIpv4PacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfIpv4PacketPartArray                                        /* partsArray */
};



/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
    GT_U32                        ethType1;
    GT_U32                        ethType2;
} prvTgfRestoreCfg;

/******************************************************************************/



/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* stored default drop mode */
static PRV_TGF_BRG_DROP_CNTR_MODE_ENT prvTgfDefDropMode = PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E;

/**
* @internal prvTgfBrgVlanIngrFltConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgVlanIngrFltConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                             isTagged  = GT_FALSE;
    GT_U32                              portIter  = 0;
    GT_U32                              vlanIter  = 0;
    GT_U32                              portCount = 0;
    GT_U32                              vlanCount = 0;
    PRV_TGF_BRG_DROP_CNTR_MODE_ENT      dropMode  = PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E;
    GT_STATUS                           rc        = GT_OK;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* clear drop counters */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

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

    /* get default counter for drop packets */
    rc = prvTgfBrgCntDropCntrModeGet(&prvTgfDefDropMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set drop counter mode in "Port not Member in VLAN Drop" */
    dropMode = PRV_TGF_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E;

    rc = prvTgfBrgCntDropCntrModeSet(dropMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, dropMode);
}

/**
* @internal prvTgfBrgVlanIngrFltTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgVlanIngrFltTrafficGenerate
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
    GT_U32          dropCount  = 0;
    GT_U16          vlanId     = 0;
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
        /* AUTODOC: enable ingress filtering on send port 0 */
        enable = GT_TRUE;

        rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* update packet */
        ((TGF_PACKET_VLAN_TAG_STC*) packetInfoPtr.partsArray[1].partPtr)->vid = PRV_TGF_DEF_VLAN_ID_CNS;

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending VALID pakets to port [%d] -------\n", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send traffic with VID 2 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get traffic on ports [0, 1] */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* get Vlan ID */
            rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[portIter], &vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* check counters from def Vlan */
            if (portIter < portsCount / 2)
            {
                if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
                {
                    /* check Rx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                                 "get another goodOctetsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                                 "get another goodPktsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                                 "get another ucPktsRcv counter than expected");
                }

                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                             "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                             "get another ucPktsSent counter than expected");
            }
            else
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0], "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0], "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0], "get another ucPktsSent counter than expected");

                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0], "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0], "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0], "get another ucPktsRcv counter than expected");
            }
        }

        /* get drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

        /* AUTODOC: check drop counter */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * i, dropCount, "get another drop counter than expected");

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* update packet */
        ((TGF_PACKET_VLAN_TAG_STC*) packetInfoPtr.partsArray[1].partPtr)->vid = PRV_TGF_INV_VLAN_ID_CNS;

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending NOT VALID pakets to port [%d] -------\n", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send traffic with VID 3 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get no traffic */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                             "get another ucPktsRcv counter than expected");
            }
            else
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0], "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0], "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0], "get another ucPktsSent counter than expected");
            }
        }

        /* get drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

        /* AUTODOC: check dropped counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * (i + 1), dropCount, "get another drop counter than expected");

        /* AUTODOC: disable ingress filtering on send port 0 */
        enable = GT_FALSE;

        rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* update packet */
        ((TGF_PACKET_VLAN_TAG_STC*) packetInfoPtr.partsArray[1].partPtr)->vid = PRV_TGF_INV_VLAN_ID_CNS;

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending VALID pakets to port [%d] -------\n", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send traffic with VID 3 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get traffic on ports [2, 3] */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* get Vlan ID */
            rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[portIter], &vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                             "get another ucPktsRcv counter than expected");
            }

            /* check counters from def Vlan */
            if (portIter >= portsCount / 2)
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                             "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                             "get another ucPktsSent counter than expected");
            }
            else if (PRV_TGF_SEND_PORT_IDX_CNS != portIter)
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0], "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0], "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0], "get another ucPktsSent counter than expected");

                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0], "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0], "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0], "get another ucPktsRcv counter than expected");
            }
        }

        /* get drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

        /* AUTODOC: check dropped counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * (i + 1), dropCount, "get another drop counter than expected");
    }
}

/**
* @internal prvTgfBrgVlanIngrFltConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanIngrFltConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_STATUS   rc        = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: clear drop counters */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

    /* get vlan count */
    vlanCount = sizeof(prvTgfVlanIdArray) / sizeof (prvTgfVlanIdArray[0]);

    /* invalidate vlan entry */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanIdArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanIdArray[vlanIter]);
    }

    /* AUTODOC: restore default drop counter mode */
    rc = prvTgfBrgCntDropCntrModeSet(prvTgfDefDropMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefDropMode);

    /* AUTODOC: disable ingress filtering */
    enable = GT_FALSE;

    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    
}








/**
* @internal prvTgfBrgVlanL2VpnIngressFilteringConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*         Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*/
GT_VOID prvTgfBrgVlanL2VpnIngressFilteringConfigSet
(
    GT_VOID
)
{

    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_STC      ruleAction = {0};

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

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

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;

        /* reset counters and set force link up */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }


    /* AUTODOC: create VLAN 2 with all ports */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 PRV_TGF_DEF_VLAN_ID_CNS,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", PRV_TGF_DEF_VLAN_ID_CNS);

    /* delete port from VLAN */
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_DEF_VLAN_ID_CNS,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VLAN_ID_CNS,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);



/* Set TTI test settings */


    /* clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));

    /* enable TTI lookup */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* set MAC mode */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_TRUE;
    ruleAction.passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    ruleAction.copyTtlFromTunnelHeader = GT_FALSE;

    ruleAction.mplsCmd = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;

    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
    ruleAction.vlanId          = PRV_TGF_INLIF_CNS;
    ruleAction.vlanCmd         = PRV_TGF_TTI_VLAN_MODIFY_ALL_E; 


    ruleAction.tunnelStart    = GT_FALSE;
    ruleAction.tunnelStartPtr = 0;

    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId  = 0;

    ruleAction.modifyUpEnable = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;

    ruleAction.userDefinedCpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;


    /* configure TTI rule */
    pattern.ipv4.common.pclId    = 1;
    pattern.ipv4.common.srcIsTrunk = GT_FALSE;
    pattern.ipv4.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    cpssOsMemCpy((GT_VOID*)pattern.ipv4.common.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(pattern.ipv4.common.mac));
    pattern.ipv4.common.vid      = PRV_TGF_DEF_VLAN_ID_CNS;
    pattern.ipv4.common.isTagged = GT_TRUE;
    pattern.ipv4.tunneltype = 4;
    pattern.ipv4.isArp = GT_FALSE;

    cpssOsMemCpy(pattern.ipv4.destIp.arIP, prvTgfPacketIpv4Part.dstAddr,
                 sizeof(pattern.ipv4.destIp.arIP));
    cpssOsMemCpy(pattern.ipv4.srcIp.arIP, prvTgfPacketIpv4Part.srcAddr,
                 sizeof(pattern.ipv4.srcIp.arIP));

    /* configure TTI rule mask */
    mask.ipv4.common.pclId = 0xFFF;
    mask.ipv4.common.srcIsTrunk = GT_TRUE;
    mask.ipv4.common.srcPortTrunk = 0xFFFF;
    cpssOsMemSet((GT_VOID*)mask.ipv4.common.mac.arEther, 0xFF,
                 sizeof(mask.ipv4.common.mac.arEther));
    mask.ipv4.common.vid = 0xFFFF;
    mask.ipv4.common.isTagged = GT_TRUE;

    mask.ipv4.tunneltype = 0xFFFF;
    mask.ipv4.isArp = GT_TRUE;
    cpssOsMemSet(mask.ipv4.srcIp.arIP,  0xFF, sizeof(mask.ipv4.srcIp.arIP));
    cpssOsMemSet(mask.ipv4.destIp.arIP, 0xFF, sizeof(mask.ipv4.destIp.arIP));


    /* set TTI rule */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

}

/**
* @internal prvTgfBrgGenVlanL2VpnIngressFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:03,
*         macSa = 00:00:00:00:00:02,
*/
GT_VOID prvTgfBrgGenVlanL2VpnIngressFilteringTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                     rc = GT_OK;
    GT_U32                        portIter = 0;
                                  
    GT_U32                        portsCount   = prvTgfPortsNum;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4PacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum,  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);



    /* check counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

}

/**
* @internal prvTgfBrgGenVlanL2VpnIngressFilteringConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenVlanL2VpnIngressFilteringConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* Invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* Invalidate VLAN entry 2 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_INV_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* restore the lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* disable the TTI lookup for IPv4 at the port */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}




