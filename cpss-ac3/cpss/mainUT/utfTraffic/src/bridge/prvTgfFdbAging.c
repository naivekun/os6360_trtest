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
* @file prvTgfFdbAging.c
*
* @brief FDB Age Bit Da Refresh
*
* @version   18
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
#include <bridge/prvTgfFdbAging.h>

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* FDB MAC entry */
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;

/* MAC Address */
static TGF_MAC_ADDR macAddr = {0,0,0,0,0,0x22};

/* destination port */
static GT_U8 destPortIdx = 3;

static GT_U32   agingTimeOut = 10;
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
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

/* aging mode */
static PRV_TGF_MAC_ACTION_MODE_ENT agingMode;

/* FDB Action status */
static GT_BOOL fdbActionEnable;

/* FDB Action mode */
static PRV_TGF_FDB_ACTION_MODE_ENT  fdbActionMode;
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbAddEntry function
* @endinternal
*
* @brief   Add FDB entry.
*
* @param[in] vlanId                   -  to be configured
* @param[in] portIdx                  - port index
* @param[in] macAddr                  - MAC address
*                                       None
*/
GT_VOID prvTgfFdbAddEntry
(
    IN GT_U16           vlanId,
    IN GT_U8            portIdx,
    IN TGF_MAC_ADDR     macAddr
)
{
    GT_STATUS                   rc        = GT_OK;
    /* Add MAC Entry */

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId       = vlanId;
    prvTgfMacEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[portIdx];
    prvTgfMacEntry.isStatic                     = GT_FALSE;
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
    prvTgfMacEntry.sourceId                     = 0;

    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");
}

/**
* @internal prvTgfFdbAgeBitDaRefreshTestInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
GT_VOID prvTgfFdbAgeBitDaRefreshTestInit
(
    IN GT_U16           vlanId,
    IN TGF_MAC_ADDR     macAddr
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

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* AUTODOC: create VLAN 2 with all ports */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);


    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:22, VLAN 2, port 3 */
    prvTgfFdbAddEntry(vlanId, destPortIdx, macAddr);

}

/**
* @internal prvTgfFdbAgeBitDaRefreshTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*/
static GT_VOID prvTgfFdbAgeBitDaRefreshTestPacketSend
(
    GT_VOID
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* AUTODOC: send packet with DA 00:00:00:00:00:22 from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfFdbAgeBitDaRefreshTestTableReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*
* @param[in] vlanId                   -  to be cleared
*                                       None
*/
static GT_VOID prvTgfFdbAgeBitDaRefreshTestTableReset
(
    IN GT_U16 vlanId
)
{
    GT_STATUS   rc;

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, vlanId);

}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbAgeBitDaRefreshConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable automatic aging.
*         Add to FDB table non-static entry with
*         MACADDR: 00:00:00:00:00:22 on port 23.
*         Change Aging time to 10 secunds (real aging will be after
*         210 second).
*         Success Criteria:
*         After 20 seconds packets captured on ports 8 and 18
*         Add to FDB table non-static entry with
*         MACADDR: 00:00:00:00:00:22 on port 23.
*         Enable Aged bit DA refresh.
*         Success Criteria:
*         After 20 seconds there will no packets captured on
*         ports 8 and 18.
*/
GT_VOID prvTgfFdbAgeBitDaRefreshConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* set trigger mode to automatic mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_AUTO_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbMacTriggerModeSet");

    agingTimeOut = 10;
    /* set aging timeout 10sec */
    rc = prvTgfBrgFdbAgingTimeoutSet(agingTimeOut);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbAgingTimeoutSet");

    /* update the aging timeout according to the granularity of the device */
    rc = prvTgfBrgFdbAgingTimeoutGet(&agingTimeOut);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbAgingTimeoutSet");

    /* set Actions enable */
    rc = prvTgfBrgFdbActionsEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionsEnableSet");

    /* AUTODOC: enable automatic aging */
    rc = prvTgfBrgFdbActionModeSet(PRV_TGF_FDB_ACTION_AGE_WITH_REMOVAL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionModeSet");

    /* set VLAN entry, TPID global tables */
    prvTgfFdbAgeBitDaRefreshTestInit(PRV_TGF_VLANID_CNS, macAddr);
}

/**
* @internal prvTgfFdbAgeBitDaRefreshTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfFdbAgeBitDaRefreshTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portsCount = prvTgfPortsNum;
    GT_U32       portIter   = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32       expectedGoodPctsValue;
    GT_U32       ii,sleepTime;


    /* AUTODOC: GENERATE TRAFFIC: */

    /***************************************/
    /* AUTODOC: Phase 1 - auto-aging check */
    /***************************************/

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    sleepTime = (agingTimeOut * 2) + 5;
    PRV_UTF_LOG1_MAC("======= : start sleep for [%d] seconds for aging testing =======\n",sleepTime);
    for (ii = 0; ii < sleepTime; ii++)
    {
        cpssOsTimerWkAfter(1000);
        utfPrintKeepAlive();
    }
    PRV_UTF_LOG0_MAC("======= : sleep ended =======\n");

    /* send packet to check auto-aging - flooding unknown UC is expected */
    prvTgfFdbAgeBitDaRefreshTestPacketSend();

    /* AUTODOC: check auto-aging - flooding unknown UC is expected */
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
        }

        expectedGoodPctsValue = prvTgfBurstCount;

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /***************************************/
    /* AUTODOC: Phase 2 - DA refresh check */
    /***************************************/

    /* AUTODOC: set DA age bit enable */
    rc = prvTgfBrgFdbAgeBitDaRefreshEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbAgeBitDaRefreshEnableSet: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: add FDB non-static entry with MAC 00:00:00:00:00:22, VLAN 2, port 3 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* sleep for half aging timeout */
    sleepTime = agingTimeOut;
    PRV_UTF_LOG1_MAC("======= : start sleep for [%d] seconds for aging testing =======\n",sleepTime);
    for (ii = 0; ii < sleepTime; ii++)
    {
        cpssOsTimerWkAfter(1000);
        utfPrintKeepAlive();
    }
    PRV_UTF_LOG0_MAC("======= : sleep ended =======\n");

    /* AUTODOC: send packet to avoid MAC entry aging */
    prvTgfFdbAgeBitDaRefreshTestPacketSend();

    sleepTime = agingTimeOut;
    PRV_UTF_LOG1_MAC("======= : start sleep for [%d] seconds for aging testing =======\n",sleepTime);
    for (ii = 0; ii < sleepTime; ii++)
    {
        cpssOsTimerWkAfter(1000);
        utfPrintKeepAlive();
    }
    PRV_UTF_LOG0_MAC("======= : sleep ended =======\n");

        /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfFdbAgeBitDaRefreshTestPacketSend();

    /* AUTODOC: check DA refresh */
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
        }

        if ((portIter == destPortIdx) || (PRV_TGF_SEND_PORT_IDX_CNS == portIter))
        {
            expectedGoodPctsValue = prvTgfBurstCount;
        }
        else
        {
            expectedGoodPctsValue = 0;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

}

/**
* @internal prvTgfFdbAgeBitDaRefreshConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAgeBitDaRefreshConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */

    /* set trigger mode to triggered mode */
    prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);

    /* FDB Actions are enabled by default */
    /*prvTgfBrgFdbActionsEnableSet(GT_FALSE); */

    /* AUTODOC: set DA age bit disable */
    prvTgfBrgFdbAgeBitDaRefreshEnableSet(GT_FALSE);

    prvTgfFdbAgeBitDaRefreshTestTableReset(PRV_TGF_VLANID_CNS);

    /* restore aging timeout 300 sec */
    prvTgfBrgFdbAgingTimeoutSet(300);

}


/**
* @internal prvTgfFdbAgingInMultiPortGroupDeviceConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*/
GT_VOID prvTgfFdbAgingInMultiPortGroupDeviceConfigurationSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    /* state that the test can use 'per port group' indication */
    usePortGroupsBmp = GT_TRUE;
}


/**
* @internal agingInMultiPortGroupDeviceTrafficGenerate function
* @endinternal
*
* @brief   for ports :
*         a.    Into port group 0 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 0.
*         b.    Into port group 1 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 1.
*         c.    Into port group 2 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 2.
*         d.    ... in the same logic do for port groups 3..6
*         e.    Into port group 7 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 7.
*         for trunks :
*         Use next 8 trunks for the test: 0xC,0x1C,0x2C,0x3C,0x4C,0x5C,0x6C,0x7C.
*         a.    Into port group 0 : Set 8 FDB entries associated with trunks (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got 8 AA (for each and every entry).
*         b.    ... in the same logic do for port groups 2..6
*         c.    Into port group 7 : Set 8 FDB entries associated with trunks (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got 8 AA (for each and every entry).
*/
static GT_VOID agingInMultiPortGroupDeviceTrafficGenerate
(
    IN CPSS_INTERFACE_TYPE_ENT  interfaceType
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    GT_U32      ii,jj,kk;
    GT_U32      fdbIndex;/* index in FDB*/
    PRV_TGF_BRG_MAC_ENTRY_STC      tmpMacEntry;
    GT_U8       testedPorts[CPSS_MAX_PORT_GROUPS_CNS] =
                                {0    , 17   , 34   , 51   ,
                                 0+64 , 17+64, 34+64, 51+64};
    GT_U8       testedTrunks[CPSS_MAX_PORT_GROUPS_CNS] =
                                {0xc  , 0x1c, 0x2c, 0x3c ,
                                 0x4c , 0x5c, 0x6c, 0x7c};
    GT_U32  goodAaMessagesNum , badAaMessagesNum;
    GT_U16  numOfPortGroups;

     /* Set the FDB entry mac address */
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 &prvTgfPacketL2Part.daMac,
                 sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.type              = interfaceType;

    PRV_UTF_LOG1_MAC("======= agingInMultiPortGroupDeviceTrafficGenerate [%s] : started =======\n",
                        (interfaceType == CPSS_INTERFACE_PORT_E) ? "on ports" : "on trunks");

    PRV_UTF_LOG0_MAC("=======  : set trigger mode =======\n");

    /* AUTODOC: set aging to 'trigger mode' */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    PRV_UTF_LOG0_MAC("=======  : register with the AppDemo to receive AA messages =======\n");
    /* AUTODOC: register with the AppDemo to receive AA messages */
    rc = prvTgfBrgFdbAuqCbRegister(CPSS_AA_E,GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* calculate number of port groups */
    rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,PRV_CPSS_DXCH_UNIT_FDB_E,&numOfPortGroups);
    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    for(ii = 0 ; ii < numOfPortGroups ; ii++)
    {
        PRV_UTF_LOG0_MAC("======= : flush FDB =======\n");
        /*c.        Flush FDB (trigger action 'delete') ' done on all port groups*/
        rc = prvTgfBrgFdbFlush(GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        currPortGroupsBmp = BIT_0 << ii;

        PRV_UTF_LOG1_MAC("=======  : set FDB entries : on port group [%d]=======\n",ii);

        for(jj = 0 ; jj < numOfPortGroups ; jj++)
        {
            macEntry.key.key.macVlan.macAddr.arEther[5] = (GT_U8)(prvTgfPacketL2Part.daMac[5] + jj);

            if(interfaceType == CPSS_INTERFACE_PORT_E)
            {
                /* port group : 0 ,1 , 2, 3 ,4 , 5, 6,  7 */
                /* port       : 0 ,17,34,51,64 ,81,99,115 */
                macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
                macEntry.dstInterface.devPort.portNum   = testedPorts[jj];
            }
            else
            {
                /* port group : 0   ,1    , 2   , 3    , 4    , 5   , 6   ,  7 */
                /* trunks     : 0xc , 0x1c, 0x2c, 0x3c , 0x4c , 0x5c, 0x6c, 0x7c*/
                macEntry.dstInterface.trunkId = testedTrunks[jj];
            }

            /* AUTODOC: add 8 FDB entries on port(1 phase)\trunk(2 phase) for port groups */
            rc = prvTgfBrgFdbMacEntrySet(&macEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        PRV_UTF_LOG0_MAC("======= : Refresh FDB to check that add is ok =======\n");

        /* AUTODOC: refresh FDB to check that add is ok */
        for(jj = 0 ; jj < numOfPortGroups ; jj++)
        {
            macEntry.key.key.macVlan.macAddr.arEther[5] = (GT_U8)(prvTgfPacketL2Part.daMac[5] + jj);

            rc = prvTgfBrgFdbMacEntryFind(&macEntry.key,&fdbIndex,NULL,NULL,&tmpMacEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            UTF_VERIFY_EQUAL0_PARAM_MAC(interfaceType,
                tmpMacEntry.dstInterface.type);
            if(interfaceType == CPSS_INTERFACE_PORT_E)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(prvTgfDevNum,
                    tmpMacEntry.dstInterface.devPort.hwDevNum);
                UTF_VERIFY_EQUAL0_PARAM_MAC(testedPorts[jj],
                    tmpMacEntry.dstInterface.devPort.portNum);
            }
            else
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(testedTrunks[jj],
                    tmpMacEntry.dstInterface.trunkId);
            }
        }

        /* AUTODOC: trigger aging twice. (to get AA messages) */
        for(kk = 0 ; kk < 2; kk++)
        {
            PRV_UTF_LOG0_MAC("======= : trigger aging loop by HW =======\n");
            /*d. trigger aging loop by HW*/
            rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
            /* wait for trigger action to end */
            rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            /* let the appdemo time to finish the AA processing */
            cpssOsTimerWkAfter(500);
            utfPrintKeepAlive();
        }

        PRV_UTF_LOG0_MAC("======= : check the AA messages received =======\n");
        /* AUTODOC: check the AA messages received */
        rc = prvTgfBrgFdbAuqMessageAaCheck(prvTgfDevNum , currPortGroupsBmp , &goodAaMessagesNum , &badAaMessagesNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if(interfaceType == CPSS_INTERFACE_PORT_E)
        {
            /* AUTODOC: check if got different number of GOOD AA messages */
            UTF_VERIFY_EQUAL0_STRING_MAC(1, goodAaMessagesNum,"got different number of GOOD AA messages\n");
        }
        else
        {
            /*check if got different number of GOOD AA messages*/
            UTF_VERIFY_EQUAL0_STRING_MAC(numOfPortGroups, goodAaMessagesNum,"got different number of GOOD AA messages\n");
        }

        /* AUTODOC: check if got any number of BAD AA messages */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, badAaMessagesNum,"got any number of BAD AA messages \n");
    }

    PRV_UTF_LOG0_MAC("=======  : unregister from the AppDemo to receive AA messages =======\n");
    /* AUTODOC: unregister from the AppDemo to receive AA messages */
    rc = prvTgfBrgFdbAuqCbRegister(CPSS_AA_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    PRV_UTF_LOG0_MAC("======= agingInMultiPortGroupDeviceTrafficGenerate : ended =======\n");

    return;
}


/**
* @internal prvTgfFdbAgingInMultiPortGroupDeviceTrafficGenerate function
* @endinternal
*
* @brief   a.    Into port group 0 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 0.
*         b.    Into port group 1 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 1.
*         c.    Into port group 2 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 2.
*         d.    ... in the same logic do for port groups 3..6
*         e.    Into port group 7 : Set 8 FDB entries associated with port N1..N8 (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got only single AA message from port group 7.
*         Use next 8 trunks for the test: 0xC,0x1C,0x2C,0x3C,0x4C,0x5C,0x6C,0x7C.
*         a.    Into port group 0 : Set 8 FDB entries associated with trunks (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got 8 AA (for each and every entry).
*         b.    ... in the same logic do for port groups 2..6
*         c.    Into port group 7 : Set 8 FDB entries associated with trunks (as dynamic entry)
*         i.    Trigger aging twice. (to get AA messages)
*         ii.    Check that got 8 AA (for each and every entry).
*/
GT_VOID prvTgfFdbAgingInMultiPortGroupDeviceTrafficGenerate
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC: Phase 1 - port mode test: */
    agingInMultiPortGroupDeviceTrafficGenerate(CPSS_INTERFACE_PORT_E);

    /* AUTODOC: Phase 2 - trunk mode test: */
    agingInMultiPortGroupDeviceTrafficGenerate(CPSS_INTERFACE_TRUNK_E);
}

/**
* @internal prvTgfFdbAgingInMultiPortGroupDeviceConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAgingInMultiPortGroupDeviceConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= : flush FDB =======\n");
    /*c.        Flush FDB (trigger action 'delete') ' done on all port groups*/
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    return;
}

/**
* @internal prvTgfFdbAuMessageFilteringConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Enable automatic aging.
*         Add to FDB table non-static entry with
*         MACADDR: 00:00:00:00:00:22 on port 23.
*         Change Aging time to 1 second (real aging will be after
*         2 seconds).
*/
GT_VOID prvTgfFdbAuMessageFilteringConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* save FDB trigger mode */
    rc = prvTgfBrgFdbMacTriggerModeGet(prvTgfDevNum, &agingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeGet");

    /* set trigger mode to trigger mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(PRV_TGF_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbMacTriggerModeSet");


    /* save Actions enable status */
    rc = prvTgfBrgFdbActionsEnableGet(&fdbActionEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionsEnableGet");


    /* set Actions enable */
    rc = prvTgfBrgFdbActionsEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionsEnableSet");

    /* save FDB action mode */
    rc = prvTgfBrgFdbActionModeGet(&fdbActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionModeGet");

    /* AUTODOC: set action mode */
    rc = prvTgfBrgFdbActionModeSet(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionModeSet");

    /* AUTODOC: refister CB with AppDemo to receive AA messages */
    rc = prvTgfBrgFdbAuqCbRegister(CPSS_AA_E,GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* set VLAN entry, TPID global tables */
    prvTgfFdbAgeBitDaRefreshTestInit(PRV_TGF_VLANID_CNS, macAddr);


}

/**
* @internal prvTgfFdbAuMessageFilteringCheck function
* @endinternal
*
* @brief   Sleep 1 sec.
*         See only 1 AA message
* @param[in] portGroupsBmp            - port groups bit map.
*                                       None
*/
static GT_VOID prvTgfFdbAuMessageFilteringCheck
(
    GT_PORT_GROUPS_BMP   portGroupsBmp
)
{
    GT_U32                          ii;
    GT_STATUS                       rc = GT_OK;
    GT_U32  goodAaMessagesNum , badAaMessagesNum;


    /* AUTODOC: trigger aging twice. (to get AA messages) */
    for(ii = 0 ; ii < 2; ii++)
    {
        PRV_UTF_LOG0_MAC("======= : trigger aging loop by HW =======\n");
        /* trigger aging loop by HW */
        rc = prvTgfBrgFdbActionStart(PRV_TGF_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        cpssOsTimerWkAfter(10);

        PRV_UTF_LOG0_MAC("======= : wait for trigger action to end =======\n");
        /* wait for trigger action to end */
        rc = prvTgfBrgFdbActionDoneWait(GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* let the appdemo time to finish the AA processing */
        cpssOsTimerWkAfter(500);
        utfPrintKeepAlive();
    }

    /* AUTODOC: check the AA messages received */
    rc = prvTgfBrgFdbAuqMessageAaCheck(prvTgfDevNum , portGroupsBmp , &goodAaMessagesNum , &badAaMessagesNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL2_STRING_MAC(1, goodAaMessagesNum, "The number of received messages is wrong: %d, %d",
                                 1, goodAaMessagesNum);



}

/**
* @internal prvTgfFdbAuMessageFilteringTrafficGenerate function
* @endinternal
*
* @brief   Sleep 1 sec.
*         See only 1 AA message
*/
GT_VOID prvTgfFdbAuMessageFilteringTrafficGenerate
(
    GT_VOID
)
{
    currPortGroupsBmp = BIT_0 << 1;

    prvTgfFdbAuMessageFilteringCheck(currPortGroupsBmp);


    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:22, VLAN 2, port 0 */
    prvTgfFdbAddEntry(PRV_TGF_VLANID_CNS, 0, macAddr);

    currPortGroupsBmp = BIT_0;

    prvTgfFdbAuMessageFilteringCheck(currPortGroupsBmp);

}
/**
* @internal prvTgfFdbAuMessageFilteringConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbAuMessageFilteringConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("=======  : unregister from the AppDemo to receive AA messages =======\n");
    /* AUTODOC: unregister from the AppDemo to receive AA messages */
    rc = prvTgfBrgFdbAuqCbRegister(CPSS_AA_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* set Actions disable */
    prvTgfBrgFdbActionsEnableSet(GT_FALSE);

    /* resore FDB trigger mode */
    rc = prvTgfBrgFdbMacTriggerModeSet(agingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error of prvTgfBrgFdbMacTriggerModeSet");


    /* restore Actions enable status */
    rc = prvTgfBrgFdbActionsEnableSet(fdbActionEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionsEnableSet");


    /* restore FDB action mode */
    rc = prvTgfBrgFdbActionModeSet(fdbActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "prvTgfBrgFdbActionModeSet");


    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    return;
}

















