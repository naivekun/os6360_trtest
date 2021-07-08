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
* @file prvTgfVlanFloodVidx.c
*
* @brief VLAN Flood VIDX
*
* @version   11
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
#include <bridge/prvTgfVlanFloodVidx.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* port bitmap for VIDX group */
static CPSS_PORTS_BMP_STC portBitmap = {{0, 0}};

static PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ENT vidxFloodMode = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x12}                /* srcMac */
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
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgVlanFloodVidxTestInit function
* @endinternal
*
* @brief   Set VLAN entry with recieved flood VIDX mode.
*
* @param[in] vlanId                   -  to be configured
*                                      vidxMode - VLAN Flood VIDX mode
*                                       None
*/
static GT_VOID prvTgfBrgVlanFloodVidxTestInit
(
    IN GT_U16                               vlanId,
    IN GT_U16                               floodVidx,
    IN PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ENT floodVidxMode
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
    vlanInfo.floodVidx            = floodVidx;
    vlanInfo.floodVidxMode        = floodVidxMode;
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
* @internal prvTgfBrgVlanFloodVidxTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*/
static GT_VOID prvTgfBrgVlanFloodVidxTestPacketSend
(
    IN TGF_MAC_ADDR prvTgfDaMac
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);


    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMac, sizeof(prvTgfPacketL2Part.daMac));

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfBrgVlanFloodVidxTestTableReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*
* @param[in] vlanId                   -  to be cleared
*                                       None
*/
static GT_VOID prvTgfBrgVlanFloodVidxTestTableReset
(
    IN GT_U16 vlanId
)
{
    GT_STATUS   rc = GT_OK;

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear VIDX 10 */
    rc = prvTgfBrgVidxEntrySet(10, NULL, NULL, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d", prvTgfDevNum);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, vlanId);

}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanFloodVidxConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure floodVidxMode = UNREG MC on VLAN entry (VID = 5).
*         Configure floodVidx = 10 on VLAN entry (VID = 5).
*         Build MC Group (devNum = 0, vidx = 10, portBitmapPtr[0] = 0x101)
*/
GT_VOID prvTgfBrgVlanFloodVidxConfigurationSet
(
    GT_VOID
)
{
    GT_U16 vidx = 10;
    vidxFloodMode = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with vidx = 10, flood mode = UNREG MC */
    prvTgfBrgVlanFloodVidxTestInit(PRV_TGF_VLANID_CNS,
                                   vidx,
                                   vidxFloodMode);

    /* clear port bitmap */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);

    /* set port bitmap */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[1]);

    /* AUTODOC: write MC Group with vidx = 10, ports [0, 1] */
    prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &portBitmap);
}

/**
* @internal prvTgfBrgVlanFloodVidxTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 01:01:01:01:01:01,
*         macSa = 00:00:00:00:00:05,
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:05,
*         Success Criteria:
*         MC Packet is captured on port 8
*         BC Packet is captured on ports 8, 18, 23
*/
GT_VOID prvTgfBrgVlanFloodVidxTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portsCount = prvTgfPortsNum;
    GT_U32       portIter   = 0;
    TGF_MAC_ADDR prvTgfDaMac = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32       expectedGoodOctetsValue;
    GT_U32       expectedGoodPctsValue;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send MC packet with DA 01:01:01:01:01:01 to port 0 */
    prvTgfBrgVlanFloodVidxTestPacketSend(prvTgfDaMac);

    /* AUTODOC: check counters - MC packet is captured on port 1 */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
        }

        if ((CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[portIter])) ||
            (vidxFloodMode == PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E))
        {
            expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
            expectedGoodPctsValue = prvTgfBurstCount;
        }
        else
        {
            expectedGoodOctetsValue = 0;
            expectedGoodPctsValue = 0;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }


    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send broadcast packet to portIdx 0 */
    prvTgfDaMac[0] = 0xFF;
    prvTgfDaMac[1] = 0xFF;
    prvTgfDaMac[2] = 0xFF;
    prvTgfDaMac[3] = 0xFF;
    prvTgfDaMac[4] = 0xFF;
    prvTgfDaMac[5] = 0xFF;

    /* AUTODOC: send BC packet with DA FF:FF:FF:FF:FF:FF to port 0 */
    prvTgfBrgVlanFloodVidxTestPacketSend(prvTgfDaMac);

    /* AUTODOC: check counters - BC packet is captured on ports 1, 2, 3 */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    portsCount = prvTgfPortsNum;

    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

}

/**
* @internal prvTgfBrgVlanFloodVidxConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanFloodVidxConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanFloodVidxTestTableReset(PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgVlanFloodVidxAllFloodedConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure floodVidxMode = ALL_FLOODED_TRAFFIC on VLAN entry (VID = 5).
*         Configure floodVidx = 4094 on VLAN entry (VID = 5).
*         Build MC Group (devNum = 0, vidx = 10, portBitmapPtr[0] = 0x101)
*/
GT_VOID prvTgfBrgVlanFloodVidxAllFloodedConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U16 vidx = 4094;
    vidxFloodMode = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* clear port bitmap */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);

    /* set port bitmap */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[1]);

    while (rc != GT_OK && vidx > 10) /* support devices that supports less then 4k VIDX */
    {
        /* AUTODOC: write MC Group with vidx = 4094, ports [0, 1] */
        rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &portBitmap);
        if (rc == GT_OK)
        {
             /* AUTODOC: create VLAN 5 with vidx = 4094, flood mode = ALL_FLOODED_TRAFFIC */
            prvTgfBrgVlanFloodVidxTestInit(PRV_TGF_VLANID_CNS,
                                       vidx,
                                       vidxFloodMode);
        }

        /* look for smaller vidx */
        vidx -= 10;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanFloodVidxAllFloodedConfigurationSet");

}

/**
* @internal prvTgfBrgVlanFloodVidxAllFloodedTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 01:01:01:01:01:01,
*         macSa = 00:00:00:00:00:05,
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:05,
*         Success Criteria:
*         MC Packet is captured on port 8
*         BC Packet is captured on port 8
*/
GT_VOID prvTgfBrgVlanFloodVidxAllFloodedTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portsCount = prvTgfPortsNum;
    GT_U32       portIter   = 0;
    GT_U32       sendIter   = 0;
    TGF_MAC_ADDR prvTgfDaMac = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32       expectedGoodOctetsValue;
    GT_U32       expectedGoodPctsValue;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    for (sendIter = 0; sendIter < 2; sendIter++)
    {
        /* AUTODOC: send MC packet with DA 01:01:01:01:01:01 to port 0 */
        prvTgfBrgVlanFloodVidxTestPacketSend(prvTgfDaMac);

        /* AUTODOC: check counters - MC packet is captured on port 1 */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
            }

            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portBitmap,prvTgfPortsArray[portIter]))
            {
                expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedGoodPctsValue = prvTgfBurstCount;
            }
            else
            {
                expectedGoodOctetsValue = 0;
                expectedGoodPctsValue = 0;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

        /* reset counters */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: send BC packet with DA FF:FF:FF:FF:FF:FF to port 0 */
        /* AUTODOC: check counters - BC packet is captured on port 1 */
        /* send broadcast packet for the next iteration */
        prvTgfDaMac[0] = 0xFF;
        prvTgfDaMac[1] = 0xFF;
        prvTgfDaMac[2] = 0xFF;
        prvTgfDaMac[3] = 0xFF;
        prvTgfDaMac[4] = 0xFF;
        prvTgfDaMac[5] = 0xFF;
    }
}

/**
* @internal prvTgfBrgVlanFloodVidxAllFloodedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanFloodVidxAllFloodedConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfBrgVlanFloodVidxTestTableReset(PRV_TGF_VLANID_CNS);
}


