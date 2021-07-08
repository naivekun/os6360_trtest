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
* @file prvTgfBrgVlanFws.c
*
* @brief VLAN Full Wire Speed tests
*
* @version   3
********************************************************************************
*/
#include <cpssCommon/private/prvCpssMath.h>
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
#include <bridge/prvTgfBrgVlanFws.h>
#include <common/tgfCommon.h>
#include <utf/private/prvUtfExtras.h>
#include <bridge/prvTgfFdbAging.h>
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port indexes */
#define PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS  1
#define PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS  2
#define PRV_TGF_TX_PORT_INDEX_CNS            3

/* VLAN0 Id */
#define PRV_TGF_VLAN0_ID_CNS  0x22
#define PRV_TGF_VLAN1_ID_CNS  0x23

/* initial burst size */
#define PRV_TGF_INITIAL_BURST_SIZE_CNS       100

/* default number of packets to send */
static GT_U16       prvTgfSavePVid0   = 1;
static GT_U16       prvTgfSavePVid1   = 1;

/******************************* Test packet **********************************/

/* The packet size should be more then 64 bytes and less then 128 bytes  to use */
/* CPSS_PKTS_65TO127_OCTETS_E and CPSS_GOOD_OCTETS_SENT_E */

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x44, 0x33, 0x22},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99}                /* srcMac */
};

/* Ethernet type part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEhertypePart =
{
    0x3456
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08 ,0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18 ,0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28 ,0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38 ,0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/************************ Double tagged packet ********************************/

/* PARTS of double tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketPartsArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEhertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* LENGTH of double tagged packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* TGF_CRC_LEN_CNS */

/* Double tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                         /* totalLen */
    (sizeof(prvTgfPacketPartsArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    prvTgfPacketPartsArray                                          /* partsArray */
};

/************************************ FWS PACKET **************************************************/

/* DATA of packet */
static GT_U8 prvTgfPayloadFWSDataArr[] =
 {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08 ,0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18 ,0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
   0x28 ,0x29, 0x2A, 0x2B, 0x2C, 0x2D,
 };

static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadFWSPart = {
    sizeof(prvTgfPayloadFWSDataArr),                       /* dataLength */
    prvTgfPayloadFWSDataArr                                /* dataPtr */
};


static TGF_PACKET_PART_STC prvTgfPacketPartsFWSArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEhertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadFWSPart}
};

/* LENGTH of double tagged packet */
#define PRV_TGF_PACKET_LEN_FWS_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadFWSDataArr)

/* TGF_CRC_LEN_CNS */

/* Double tagged PACKET to send */
static TGF_PACKET_STC  prvTgfPacketFWSInfoB =
{
    PRV_TGF_PACKET_LEN_FWS_CNS,                                         /* totalLen */
    (sizeof(prvTgfPacketPartsFWSArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    prvTgfPacketPartsFWSArray                                          /* partsArray */
};

 /****************************************FDB MAC entry and address****************************/
/*FDB MAC entry*/
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;
 
/* MAC Address */
static TGF_MAC_ADDR macAddr = {0,0,0,0,0,0x01};
/*key*/
static PRV_TGF_MAC_ENTRY_KEY_STC macEntryKey;



  
  
/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanFws2PortsVlanCreate function
* @endinternal
*
* @brief   Set VLAN entry with 2 member ports untagged.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfBrgVlanFws2PortsVlanCreate
(
    IN GT_U16                            vlanId
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

    CPSS_PORTS_BMP_PORT_SET_MAC(
        &portsMembers,prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(
        &portsMembers,prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS]);

    portsTaggingCmd.portsCmd[prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS]] =
        PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    portsTaggingCmd.portsCmd[prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS]] =
        PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    rc = prvTgfBrgVlanEntryWrite(
        prvTgfDevNum, vlanId, &portsMembers,
        &portsTagging, &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfBrgVlanFws2PortsTrafficGeneratorCfg function
* @endinternal
*
* @brief   Configure 2 ports traffic generator.
*         - two loopped back ports - port0 and port1
*         - two VLANs containg loopped back ports vid0 and vid1
*         - PVID(port0) = vid0, PVID(port1) = vid1
*/
static GT_VOID prvTgfBrgVlanFws2PortsTrafficGeneratorCfg
(
    IN GT_VOID
)
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* AUTODOC: save port0 PVID */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        &prvTgfSavePVid0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet\n");

    /* AUTODOC: save port1 PVID */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS],
        &prvTgfSavePVid1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet\n");

    /* AUTODOC: create two VLANs contain port0 and port0 */
    prvTgfBrgVlanFws2PortsVlanCreate(PRV_TGF_VLAN0_ID_CNS);
    prvTgfBrgVlanFws2PortsVlanCreate(PRV_TGF_VLAN1_ID_CNS);

    /* AUTODOC: set port0 PVID */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        PRV_TGF_VLAN0_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet\n");

    /* AUTODOC: set port1 PVID */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS],
        PRV_TGF_VLAN1_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet\n");

    /* AUTODOC: force port0 link UP and Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(
        &portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet\n");
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

    /* AUTODOC: force port1 link UP and Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(
        &portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet\n");
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

    /* AUTODOC: Disable flow control */
    rc = prvTgfPortFcHolSysModeSet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        PRV_TGF_PORT_PORT_HOL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortFcHolSysModeSet\n");
}

/**
* @internal prvTgfBrgVlanFws2PortsTrafficGeneratorRestore function
* @endinternal
*
* @brief   Restore 2 ports traffic generator configuration.
*/
static GT_VOID prvTgfBrgVlanFws2PortsTrafficGeneratorRestore
(
    IN GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* AUTODOC: set port0 and no Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_FALSE);
    PRV_UTF_VERIFY_RC1(
        rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

    /* AUTODOC: set port1 and no Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_FALSE);
    PRV_UTF_VERIFY_RC1(
        rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

    /* AUTODOC: restore port0 PVID */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        prvTgfSavePVid0);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet\n");

    /* AUTODOC: restore port1 PVID */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS],
        prvTgfSavePVid1);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet\n");

    /* AUTODOC: invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN0_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN1_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: Enable flow control */
    rc = prvTgfPortFcHolSysModeSet(
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS],
        PRV_TGF_PORT_PORT_FC_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPortFcHolSysModeSet");

    if(rc1 != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\n[TGF]: prvTgfBrgVlanFws2PortsTrafficGeneratorRestore FAILED, rc = [%d]\n",
            rc1);
    }

}

/**
* @internal prvTgfBrgVlanFwsRestore function
* @endinternal
*
* @brief   general restore configuration.
*/
static GT_VOID prvTgfBrgVlanFwsRestore
(
    IN GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    if(rc1 != GT_OK)
    {
        PRV_UTF_LOG1_MAC("\n[TGF]: prvTgfBrgVlanFwsRestore FAILED, rc = [%d]\n",
            rc1);
    }

}

/**
* @internal prvTgfBrgVlanFwsTrafficStart function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*/
static GT_VOID prvTgfBrgVlanFwsTrafficStart
(
    IN GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_STATUS rc          = GT_OK;

    /* AUTODOC: force TX port link UP and reset counters */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_TX_PORT_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_TX_PORT_INDEX_CNS];
    rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(
        &portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth\n");
    rc = tgfTrafficGeneratorPortCountersEthReset(&portInterface);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortCountersEthReset\n");

    /* AUTODOC: send burst of packets from CPU to looped back port0 */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    rc = tgfTrafficGeneratorPortTxEthTransmit(
        &portInterface, &prvTgfPacketInfo,
        PRV_TGF_INITIAL_BURST_SIZE_CNS,
        0 /*numVfd*/, NULL /*vfdArray[]*/,
        0 /*sleepAfterXCount*/,
        0 /*sleepTime*/,
        0 /*traceBurstCount*/,
        GT_TRUE/*loopbackEnabled*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortTxEthTransmit\n");
}

/**
* @internal prvTgfBrgVlanFwsTrafficStop function
* @endinternal
*
* @brief   Function stops the two port traffic generator.
*/
static GT_VOID prvTgfBrgVlanFwsTrafficStop
(
    IN GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_STATUS rc          = GT_OK;

    /* AUTODOC: disable port0 Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

    /* AUTODOC: disable port1 Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

    /* time to drop all packets */
    cpssOsTimerWkAfter(0x10);

    /* AUTODOC: enable port0 Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT0_INDEX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

    /* AUTODOC: enable port1 link UP and Loop Back */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  =
        prvTgfDevsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    portInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_LOOPED_BACK_PORT1_INDEX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(
        &portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet\n");

}

/**
* @internal prvTgfBrgVlanFwsTrafficTest function
* @endinternal
*
* @brief   Function tests Vlan member add/remove under traffic.
*
* @param[in] isTagged                 - GT_TRUE/GT_FALSE, to set the port as tagged/untagged
* @param[in] iterationNum             - number of iterations adding/removing port member
*                                       None
*/
static GT_VOID prvTgfBrgVlanFwsTrafficTest
(
    IN GT_BOOL              isTagged,
    IN GT_U32               iterationNum
)
{
    GT_U64                         cntrPacketsSent;
    GT_U64                         cntrOctetsSent;
    GT_STATUS                      rc;
    GT_U32                         i;
    GT_U32                         expectedPacketLength;
    GT_U64                         expectedOctetsSent;

    /* AUTODOC: Start Wire Speed traffic  */
    prvTgfBrgVlanFwsTrafficStart();

    for (i = 0; (i < iterationNum); i++)
    {
        /* AUTODOC: Add TX port to VLAN  */
        rc = prvTgfBrgVlanMemberAdd(
            prvTgfDevsArray[PRV_TGF_TX_PORT_INDEX_CNS],
            PRV_TGF_VLAN0_ID_CNS,
            prvTgfPortsArray[PRV_TGF_TX_PORT_INDEX_CNS],
            isTagged);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfBrgVlanMemberAdd\n");

        /* AUTODOC: Remove TX port from VLAN  */
        rc = prvTgfBrgVlanMemberRemove(
            prvTgfDevsArray[PRV_TGF_TX_PORT_INDEX_CNS],
            PRV_TGF_VLAN0_ID_CNS,
            prvTgfPortsArray[PRV_TGF_TX_PORT_INDEX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfBrgVlanMemberRemove\n");
    }

    /* AUTODOC: Stop Wire Speed traffic  */
    prvTgfBrgVlanFwsTrafficStop();

    /* AUTODOC: Read TX Port MAC Counters (sent packets) */
    rc = prvTgfPortMacCounterGet(
        prvTgfDevsArray[PRV_TGF_TX_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_TX_PORT_INDEX_CNS],
        CPSS_PKTS_65TO127_OCTETS_E,
        &cntrPacketsSent);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortMacCounterGet\n");
    PRV_UTF_LOG2_MAC(
        "Received CPSS_PKTS_65TO127_OCTETS_E High: 0x%8.8X, Low: 0x%8.8X \n",
        cntrPacketsSent.l[1], cntrPacketsSent.l[0]);


    /* AUTODOC: Read TX Port MAC Counters (sent octets) */
    rc = prvTgfPortMacCounterGet(
        prvTgfDevsArray[PRV_TGF_TX_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_TX_PORT_INDEX_CNS],
        CPSS_GOOD_OCTETS_SENT_E,
        &cntrOctetsSent);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPortMacCounterGet\n");
    PRV_UTF_LOG2_MAC(
        "Received CPSS_GOOD_OCTETS_SENT_E High: 0x%8.8X, Low: 0x%8.8X \n",
        cntrOctetsSent.l[1], cntrOctetsSent.l[0]);

    /* AUTODOC: Calculate expected amount of sent Octetes */
    expectedPacketLength =
        prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS
        + ((isTagged) ? TGF_VLAN_TAG_SIZE_CNS : 0);

    expectedOctetsSent =
        prvCpssMathMul64(cntrPacketsSent.l[0], expectedPacketLength);
    expectedOctetsSent.l[1] += cntrPacketsSent.l[1] * expectedPacketLength;

    /* AUTODOC: Check amount of sent Octetes */
    UTF_VERIFY_EQUAL0_STRING_MAC(
        expectedOctetsSent.l[0], cntrOctetsSent.l[0],
        "expectedOctetsSent.l[0], cntrOctetsSent.l[0]\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        expectedOctetsSent.l[1], cntrOctetsSent.l[1],
        "expectedOctetsSent.l[1], cntrOctetsSent.l[1]\n");
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanFwsTest function
* @endinternal
*
* @brief   Function tests Vlan member add/remove under traffic.
*/
GT_VOID prvTgfBrgVlanFwsTest
(
    IN GT_VOID
)
{
    /* AUTODOC: Initial configuration */
    prvTgfBrgVlanFws2PortsTrafficGeneratorCfg();

    /* AUTODOC: add/remove TX port as tagged VLAN member under Wire Speed traffic
       1024 times and check sent packets and sent octets amounts */
    prvTgfBrgVlanFwsTrafficTest(
        GT_TRUE /*isTagged*/,
        1024 /*iterationNum*/);

    /* AUTODOC: add/remove TX port as untagged VLAN member under Wire Speed traffic
       1024 times and check sent packets and sent octets amounts */
    prvTgfBrgVlanFwsTrafficTest(
        GT_FALSE /*isTagged*/,
        1024 /*iterationNum*/);

    /* AUTODOC: Restore configuration */
    prvTgfBrgVlanFws2PortsTrafficGeneratorRestore();
    prvTgfBrgVlanFwsRestore();
}

extern GT_STATUS cpssDxChBrgFdbCount_debug
(
    IN  GT_U8   devNum
);

/**
* @internal prvTgfFdbAutoAgingConfigurationSet function
* @endinternal
*
* @brief   configuration of automatic aging daemon
*/

static GT_VOID prvTgfFdbAutoAgingConfigurationSet
(
    IN    GT_U32    timeout
)
{
    GT_STATUS   rc;

    /* set trigger mode to automatic mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_AUTO_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbMacTriggerModeSet");

   /* set aging timeout 10sec */
    rc = prvTgfBrgFdbAgingTimeoutSet(timeout);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbAgingTimeoutSet");

   /* set Actions enable */
    rc = prvTgfBrgFdbActionsEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionsEnableSet");

    /* AUTODOC: enable automatic aging */
    rc = prvTgfBrgFdbActionModeSet(PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionModeSet");

}

/**
* @internal prvTgfFdbAutoAgingAddEntryAndRefresh function
* @endinternal
*
* @brief   check aging daemon in automatic mode -
*         edd entry, wait for time out anf verify that the entry not exist
*/
static GT_VOID prvTgfFdbAutoAgingAddEntryAndRefresh
(
    IN    GT_U32                    sleepTime,
    IN    GT_U16                    vlanId ,
    IN    GT_U8                     portIndex ,
    IN    PRV_TGF_MAC_ENTRY_KEY_STC *macEntryKey

)
{
    GT_STATUS   rc;
    GT_U32      ii;

    /* add FDB entry with MAC 00:00:00:00:00:22, VLAN 2, port 82 */
    prvTgfFdbAddEntry(vlanId, portIndex, macAddr);

    PRV_UTF_LOG1_MAC("======= : sleep %d sec =======\n",sleepTime);

    for (ii = 0; ii < sleepTime; ii++)
    {
        cpssOsTimerWkAfter(1000);
        utfPrintKeepAlive();
    }

    PRV_UTF_LOG0_MAC("======= : finish sleep =======\n");

    /* check that entry exists */
    rc = prvTgfBrgFdbMacEntryFind(macEntryKey,NULL,NULL,NULL,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc,"auto aging before traffic: prvTgfBrgFdbMacEntryFind: rc %d",rc);
}


/**
* @internal prvTgfFdbFlushAddEntryAndRefresh function
* @endinternal
*
* @brief   check flush fdb -
*         Add entry, do flush fdb verify that the entry not exist
*/
static GT_VOID prvTgfFdbFlushAddEntryAndRefresh
(
    IN    GT_U16                    vlanId ,
    IN    GT_U8                     portIndex , 
    IN    PRV_TGF_MAC_ENTRY_KEY_STC *macEntryKey
)
{
    GT_STATUS   rc;

    /*add FDB entry with MAC 00:00:00:00:00:22, VLAN 2, port 82 */
    prvTgfFdbAddEntry(vlanId, portIndex, macAddr);

    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* check that entry exists */
    rc = prvTgfBrgFdbMacEntryFind(macEntryKey,NULL,NULL,NULL,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc," flush , no traffic: prvTgfBrgFdbMacEntryFind: rc %d",rc);


}





/**
* @internal prvTgfFdbTrigAgingAddEntryAndRefresh function
* @endinternal
*
* @brief   check aging daemon in trigger mode -
*         edd entry, wait for time out anf verify that the entry not exist
*/

static GT_VOID prvTgfFdbTrigAgingAddEntryAndRefresh
(
    IN    GT_U32                    sleepTime,
    IN    GT_U16                    vlanId ,
    IN    GT_U8                     port , 
    IN    PRV_TGF_MAC_ENTRY_KEY_STC *macEntryKey
)
{
    GT_STATUS   rc;
    GT_U32      ii;

    prvTgfFdbAddEntry(vlanId, port, macAddr);

    for (ii = 0 ; ii < 2; ii++) 
    {
        /* trigger aging loop by HW*/
        rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        PRV_UTF_LOG0_MAC("======= : aging trigger: after action start, before calling BrgFdbActionDoneWait =======\n");

        /* wait for trigger action to end */
        rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        PRV_UTF_LOG0_MAC("======= : aging trigger: after calling BrgFdbActionDoneWait =======\n");
    }
    PRV_UTF_LOG0_MAC("======= : aging trigger: finish the loop, try to sleep =======\n");

    cpssOsTimerWkAfter(sleepTime);

    PRV_UTF_LOG0_MAC("======= : aging trigger: after sleep, try to find entry =======\n");

   /* check that entry exists */
    rc = prvTgfBrgFdbMacEntryFind(macEntryKey,NULL,NULL,NULL,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc,"IN FIRST TIME BEFORE TRAFFIC: prvTgfBrgFdbMacEntryFind: rc %d",rc ); 
}


/**
* @internal prvTgfFdbFwsGenerateTraffic function
* @endinternal
*
* @brief   Generate full wire speed and print the rate
*/
static GT_VOID prvTgfFdbFwsGenerateTraffic
(
    IN GT_VOID 
)
{
    GT_STATUS   rc;
    GT_U16    firstVid   = 5;                               /* first VLAN id for the test */
    GT_U32    packetSize = PRV_TGF_PACKET_LEN_FWS_CNS + 4;
    GT_U32    portRxRateArr[CPSS_MAX_PORTS_NUM_CNS];        /* rate of packets per port */
    

    cpssOsMemSet(portRxRateArr, 0, sizeof(portRxRateArr));

    PRV_UTF_LOG0_MAC("======= : generate FWS traffic =======\n");
    /* Create FWS traffic*/
    rc = prvTgfFWSLoopConfigTrafficGenerate(firstVid, &prvTgfPacketFWSInfoB, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", firstVid);

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    /* this test will always fail when FdbMaxLookupLenSet 8
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficCheck : size %d",packetSize );*/

#ifdef IMPL_GALTIS
    PRV_UTF_LOG0_MAC("======= : Try to access FDB =======\n");
    /*access to FDB*/
    rc = cpssDxChBrgFdbCount_debug(prvTgfDevNum); 
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChBrgFdbCount_debug\n");

    PRV_UTF_LOG0_MAC("======= :  succeeded to access FDB =======\n");
#endif /*IMPL_GALTIS*/

 }


/*******************************************************************************
* prvTgfBrgFDBFwsTraffic
*
* DESCRIPTION:
*       Full wire speed FDB access test:
*       check the Performance of FDB access and Aging daemon under FWS
*       check 3 type of aging:
*       triger aging, automatic aging and flush fdb.
*       for each one:
*       1. try add entry and verify removel without traffic
*       2. Generate FWS traffic and check again
*       3. Stop traffic and do reset
*              
*
* INPUTS:
*
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfBrgFDBFwsTraffic 
(
  IN GT_VOID 
)   
{
    GT_STATUS       rc;
    GT_U32      timeout    = 4;
    GT_U16      vlanId     = 2;                              /*vlan id*/
    GT_U8       port = 3;
    GT_U32      sleepTime;
    GT_U32      origTimeOut,origFdbMaxLookupLen ;
    PRV_TGF_MAC_ACTION_MODE_ENT origAction;
  
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry)); /* NOT NEED*/

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));
    macEntryKey.entryType            = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId   = vlanId;

    sleepTime = (2*timeout)+ 2;

    cpssDxChBrgFdbMaxLookupLenGet(prvTgfDevNum,&origFdbMaxLookupLen);

    prvTgfBrgFdbMacTriggerModeGet(prvTgfDevNum,&origAction);

    /*generate real stress on FDB memory for maximum FWS  */
    cpssDxChBrgFdbMaxLookupLenSet(prvTgfDevNum,8);
    /*check agigng: set fdb entry, set agigng timeout, sleep and make sure that aging removed it*/
    /* update the aging timeout according to the granularity of the device */
    rc = prvTgfBrgFdbAgingTimeoutGet(&origTimeOut);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbAgingTimeoutSet");

    /***************************************/
    /* AUTODOC: Phase 1 -  Triger Aging check */
    /***************************************/
  
    PRV_UTF_LOG0_MAC("======= : check triger aging while FWS =======\n");

    /* set aging to 'trigger mode' */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
 
    rc = prvTgfBrgFdbAgingTimeoutSet(timeout);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbAgingTimeoutSet");

    prvTgfFdbTrigAgingAddEntryAndRefresh(sleepTime,vlanId,port,&macEntryKey);

    prvTgfFdbFwsGenerateTraffic();

    prvTgfFdbTrigAgingAddEntryAndRefresh(sleepTime,vlanId,port,&macEntryKey);

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop\n"); 

    prvTgfFdbTrigAgingAddEntryAndRefresh(sleepTime,vlanId,port,&macEntryKey);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    
    PRV_UTF_LOG0_MAC("======= : finish triger aging while FWS =======\n");
 
    /***************************************/
    /* AUTODOC: Phase 2 -  auto Aging check */
    /***************************************/

    PRV_UTF_LOG0_MAC("======= : check auto aging while FWS =======\n");

    /* set auto agigng configuration*/
    prvTgfFdbAutoAgingConfigurationSet(timeout);

    /*check auto agigng without traffic*/
    prvTgfFdbAutoAgingAddEntryAndRefresh(sleepTime,vlanId,port,&macEntryKey);
   
    /*start traffic*/
    prvTgfFdbFwsGenerateTraffic();

    /*check auto agigng under traffic*/
    prvTgfFdbAutoAgingAddEntryAndRefresh(sleepTime,vlanId,port,&macEntryKey);

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop\n");

    /* After traffic stopped ... make sure that Daemon is OK */
    prvTgfFdbAutoAgingAddEntryAndRefresh(sleepTime,vlanId,port,&macEntryKey);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    

    PRV_UTF_LOG0_MAC("======= : finish auto aging while FWS =======\n");
    
     /***************************************/
    /* AUTODOC: Phase 3 -  fdb flush check */
    /***************************************/  
    
    PRV_UTF_LOG0_MAC("======= : check flush FDB while FWS =======\n"); 

    prvTgfFdbFlushAddEntryAndRefresh(vlanId,port,&macEntryKey);
    
    prvTgfFdbFwsGenerateTraffic();

    prvTgfFdbFlushAddEntryAndRefresh(vlanId,port,&macEntryKey);

    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop\n");
    
    /* After traffic stopped ... make sure that Daemon is OK */
    prvTgfFdbFlushAddEntryAndRefresh(vlanId,port,&macEntryKey);
    
    PRV_UTF_LOG0_MAC("======= : finish flush FDB while FWS  =======\n");

    /*------------------- restore ------------------------------*/

    cpssDxChBrgFdbMaxLookupLenSet(prvTgfDevNum,origFdbMaxLookupLen);

    /* stop traffic and restore port/vlan config */
    prvTgfFWSRestore();

    rc = prvTgfBrgFdbAgingTimeoutSet(origTimeOut);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbAgingTimeoutSet");

    /* set trigger mode to triggered mode */
    prvTgfBrgFdbMacTriggerModeSet(origAction);
   

}




