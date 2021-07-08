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
* @file prvTgfTunnelTermTSPassengerTypeDualLookup.c
*
* @brief Tunnel: Test Dual Lookup Tunnel Start Passenger Type
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermTSPassengerTypeDualLookup.h>


/**************************** packet Ethernet **********************************/

#define PRV_TGF_VLANID_5_CNS 5
#define PRV_TGF_VLANID_6_CNS 6

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS 0

/* egress port index to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS 3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* the TTI Rule indexes */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)
#define PRV_TGF_TTI1_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1, 0)

/* tunnel start pointer */
#define PRV_TGF_TS_PTR_CNS          0

/* tunnel start entry index */
#define PRV_TGF_TS_INDEX_CNS        0

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthernetVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x44,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketEthernetPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketEthernetVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET Ethernet to send */
static TGF_PACKET_STC prvTgfPacketEthernetInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketEthernetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthernetPartArray                                        /* partsArray */
};

static PRV_TGF_TTI_RULE_UNT      pattern;
static PRV_TGF_TTI_RULE_UNT      mask;
static PRV_TGF_TTI_ACTION_2_STC  ruleAction;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal prvTgfTunnelTermTSPassengerTypeDualLookupBridgeConfigSet function
* @endinternal
*
* @brief   Set bridge configuration
*/
static GT_VOID prvTgfTunnelTermTSPassengerTypeDualLookupBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc         = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermTSPassengerTypeDualLookupTtiRuleConfigurationSet function
* @endinternal
*
* @brief   Set basic rule configuration
*/
static GT_VOID prvTgfTunnelTermTSPassengerTypeDualLookupTtiRuleConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT  tsConfig;

    /* AUTODOC: clear TS entry */
    cpssOsMemSet((GT_VOID*) &tsConfig, 0, sizeof(tsConfig));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TS_INDEX_CNS,
                                   CPSS_TUNNEL_GENERIC_IPV4_E, &tsConfig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: clear TTI Action Entry, Mask and Pattern */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* AUTODOC: enable TTI lookup for port 0, key PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern and Mask to match any packets */

    /*
        AUTODOC: set TTI0 rule with redirect command Redirect to Egress
                 this rule will change packet byte 17 to '0x66'
    */
    ruleAction.tunnelStart                     = GT_TRUE;
    ruleAction.tunnelStartPtr                  = PRV_TGF_TS_PTR_CNS;
    ruleAction.tsPassengerPacketType           = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    ruleAction.redirectCommand                 = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ruleAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.tag0VlanId                      = 0x06;

    ruleAction.bridgeBypass = GT_TRUE;

    ruleAction.continueToNextTtiLookup = GT_TRUE;

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /*
        AUTODOC: set TTI1 rule with redirect command NO_REDIRECT
                 this rule will change packet byte 17 to '0x58' or '0x3e' incase IP packet length is 0x2A
    */
    ruleAction.continueToNextTtiLookup         = GT_FALSE;

    ruleAction.redirectCommand                 = PRV_TGF_TTI_NO_REDIRECT_E;
    ruleAction.tunnelStartPtr                  = PRV_TGF_TS_PTR_CNS;
    ruleAction.tsPassengerPacketType           = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI1_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermTSPassengerTypeDualLookupTtiRedirectCmd function
* @endinternal
*
* @brief   Sets the TTI rule redirect command
*
* @param[in] redirectCmd              - redirect command
*                                      ruleIindex  - the index of a rule to set
*                                       None
*/
GT_VOID prvTgfTunnelTermTSPassengerTypeDualLookupTtiRedirectCmd
(
    GT_BOOL redirectCmd,
    GT_U32  ruleIndex
)
{
    GT_STATUS rc = GT_OK;

    ruleAction.redirectCommand = redirectCmd ? PRV_TGF_TTI_REDIRECT_TO_EGRESS_E
                                             : PRV_TGF_TTI_NO_REDIRECT_E;

    if (ruleIndex==PRV_TGF_TTI0_INDEX_CNS)
    {
        ruleAction.continueToNextTtiLookup = GT_TRUE;
    }
    else
    {
        ruleAction.continueToNextTtiLookup = GT_FALSE;
    }
    /* AUTODOC: add TTI rule <ruleIndex> with redirect command <redirectCmd> */
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_ETH_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prvTgfTunnelTermTSPassengerTypeDualLookupTrafficOnPort function
* @endinternal
*
* @brief   Checks packet on egress
*/
GT_VOID prvTgfTunnelTermTSPassengerTypeDualLookupTrafficOnPort
(
    GT_U32   capturedPortNum,
    GT_BOOL expectedSecondTtiTs,
    GT_BOOL incorrectIpTotalLen
)
{
    GT_STATUS rc          = GT_FAIL;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];
    GT_U32                  numTriggers  = 0;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = capturedPortNum;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* Prepare VFD array entries for VLAN tag */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 17;
    vfdArray[0].cycleCount = sizeof(GT_U8);
    vfdArray[0].modeExtraInfo=0;
    vfdArray[0].incValue=1;
    if (incorrectIpTotalLen==GT_TRUE)
    {
        /* the length of the passenger 'ipv4' (0x2A) may suggest
           length shorter then length till end of packet.
           this is actual HW behavior.
        */
        vfdArray[0].patternPtr[0] = expectedSecondTtiTs ? 0x3e : 0x66;
    }
    else
    {
        /* the length of the packet (0x44) is the length untill the end of the packet*/
        vfdArray[0].patternPtr[0] = expectedSecondTtiTs ? 0x58 : 0x66;
    }
    vfdArray[0].cycleCount=1;

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1,
                                                        vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                               "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                               " port = %d, rc = 0x%02X\n",
                               portInterface.devPort.portNum, rc);

    /* check if captured packet has all correct vfd's */
    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers, "Got wrong num of triggers: %d\n", numTriggers);

}

/**
* @internal prvTgfTunnelTermTSPassengerTypeDualLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate Traffic
*
* @param[in] expectedSecondTtiTs      - GT_TRUE: expect TTI1 Tunnel Start Passenger Type
*                                      GT_FALSE: expect TTI0 Tunnel Start Passenger Type
* @param[in] incorrectIpTotalLen      - GT_TRUE:sending the IP passenger with an incorrect total length
*                                      GT_FALSE:sending the IP passenger with correct total length
*                                       None
*/
static GT_VOID prvTgfTunnelTermTSPassengerTypeDualLookupTrafficGenerate
(
    IN GT_BOOL  expectedSecondTtiTs,
    IN GT_BOOL  incorrectIpTotalLen
)
{
    GT_STATUS rc               = GT_FAIL;
    GT_U32    portIter         = 0;
    GT_U32    prvTgfBurstCount = 1;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          goodOctets  = 0;

    /* AUTODOC: GENERATE TRAFFIC */

    /*  Reset all Ethernet port's counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    if (incorrectIpTotalLen==GT_TRUE)
    {
        prvTgfPacketIpv4Part.totalLen = 0x2A;
    }
    else
    {
        prvTgfPacketIpv4Part.totalLen = 0x44;
    }

    /* AUTODOC: send 1 ethernet packet from port 0 with: VID=5 */

    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfPacketEthernetInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketEthernetPartArray) / sizeof(prvTgfPacketEthernetPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketEthernetPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");


    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (expectedSecondTtiTs==GT_TRUE)
        {
            goodOctets = (packetSize - TGF_VLAN_TAG_SIZE_CNS + TGF_CRC_LEN_CNS + TGF_IPV4_HEADER_SIZE_CNS) * prvTgfBurstCount;
        }
        else
        {
            goodOctets = (packetSize - TGF_VLAN_TAG_SIZE_CNS + TGF_CRC_LEN_CNS + (TGF_ETHERTYPE_SIZE_CNS + TGF_L2_HEADER_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS /*ts part*/)) * prvTgfBurstCount;
        }
        /* AUTODOC: verify packet on port 3 */
        if(prvTgfPortsArray[portIter]==prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS])
        {
            /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }
        else
        {
            if(prvTgfPortsArray[portIter]==prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS])
            {
                expectedCntrs.goodOctetsSent.l[0] = goodOctets;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = goodOctets;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
            }
            else
            {
                 /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
            }
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    prvTgfTunnelTermTSPassengerTypeDualLookupTrafficOnPort(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], expectedSecondTtiTs,incorrectIpTotalLen);
}

/**
* @internal prvTgfTunnelTermTSPassengerTypeDualLookupConfigurationRestore function
* @endinternal
*
* @brief   Restore base configuration
*/
static GT_VOID prvTgfTunnelTermTSPassengerTypeDualLookupConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */

    GT_STATUS          rc = GT_OK;
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for port 0, key PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");
}


/**
* @internal prvTgfTunnelTermTSPassengerTypeDualLookup function
* @endinternal
*
* @brief   Test main func
*/
GT_VOID prvTgfTunnelTermTSPassengerTypeDualLookup
(
    GT_VOID
)
{
    /* AUTODOC: build ethernet packet with vid 5 */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    prvTgfTunnelTermTSPassengerTypeDualLookupBridgeConfigSet();
    /*------------------------------------------------------------*/

    /* AUTODOC: add TTI0 rule with redirect command Redirect to Egress and
                ts passenger type ETHERNET
                It will change packet byte 17 to '0x66'
                add TTI1 rule with redirect command NO_REDIRECT and
                ts passenger type OTHER
                It will change packet byte 17 to '0x58' or to '0x3e' incase IP packet length is 0x2A
    */
    prvTgfTunnelTermTSPassengerTypeDualLookupTtiRuleConfigurationSet();

    /* AUTODOC: send packet and expect TTI match and TTI0 Tunnel Start Passenger Type taken */
    prvTgfTunnelTermTSPassengerTypeDualLookupTrafficGenerate(GT_FALSE,GT_FALSE);
    /*------------------------------------------------------------*/

    /* AUTODOC: change TTI0 Cmd to NO_REDIRECT */
    /* AUTODOC: change TTI1 Cmd to REDIRECT_TO_EGRESS */
    prvTgfTunnelTermTSPassengerTypeDualLookupTtiRedirectCmd(GT_FALSE, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermTSPassengerTypeDualLookupTtiRedirectCmd(GT_TRUE, PRV_TGF_TTI1_INDEX_CNS);

    /* AUTODOC: send packet and expect TTI match and TTI1 Tunnel Start Passenger Type taken */
    prvTgfTunnelTermTSPassengerTypeDualLookupTrafficGenerate(GT_TRUE,GT_FALSE);
    /*------------------------------------------------------------*/

    /* AUTODOC: change TTI0 Cmd to REDIRECT_TO_EGRESS */
    prvTgfTunnelTermTSPassengerTypeDualLookupTtiRedirectCmd(GT_TRUE, PRV_TGF_TTI0_INDEX_CNS);

    /* AUTODOC: send packet and expect TTI match and TTI1 Tunnel Start Passenger Type taken */
    prvTgfTunnelTermTSPassengerTypeDualLookupTrafficGenerate(GT_TRUE,GT_FALSE);
    /*------------------------------------------------------------*/

    /* AUTODOC: send packet with incorrect ipv4 total length
       and expect TTI match and TTI1 Tunnel Start Passenger Type taken */
    prvTgfTunnelTermTSPassengerTypeDualLookupTrafficGenerate(GT_TRUE,GT_TRUE);
    /*------------------------------------------------------------*/

    /* AUTODOC: test configuration restore */
    prvTgfTunnelTermTSPassengerTypeDualLookupConfigurationRestore();
}



