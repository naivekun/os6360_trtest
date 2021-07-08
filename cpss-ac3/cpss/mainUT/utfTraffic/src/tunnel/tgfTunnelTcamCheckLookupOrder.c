/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfTunnelTermDualLookup.c
*
* DESCRIPTION:
*       Tunnel: TCAM check lookup order
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>
#include <tunnel/tgfTunnelTcamCheckLookupOrder.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* VLAN Id 5*/
#define PRV_TGF_VLANID_5_CNS 5

/* VLAN Id 6*/
#define PRV_TGF_VLANID_6_CNS 6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS 0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS 3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* maximal relative index for TTI0 (TTI1 first index) */
#define PRV_TGF_TTI0_SIZE_CNS 5120

/* maximal relative index for TTI1 */
#define PRV_TGF_TTI1_SIZE_CNS 6144

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x54

/*  IPv4 packet to send */
static TGF_PACKET_STC prvTgfIpv4PacketInfo;

/* index of current iteration. Used as: ruleIndex, macDa */
static GT_U32 currentStep = 0;

/* TTI rule */
static PRV_TGF_TTI_ACTION_2_STC  ruleAction;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/


/**
* @internal tgfTunnelTcamCheckLookupOrderBridgeConfigSet function
* @endinternal
*
* @brief   Set bridge configuration
*/
GT_VOID tgfTunnelTcamCheckLookupOrderBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc         = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS,
                                           prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS,
                                           prvTgfPortsArray + 2, NULL,
                                           tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d",
                                 prvTgfDevNum);
}

/**
* @internal tgfTunnelTcamCheckLookupOrderBuildPacket function
* @endinternal
*
* @brief   Build a packet
*/
GT_VOID tgfTunnelTcamCheckLookupOrderBuildPacket
(
   GT_VOID
)
{
    GT_STATUS               rc              = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr    = {0};
    TGF_PACKET_VLAN_TAG_STC *vlanTagPartPtr = {0};
    TGF_PACKET_STC          packetInfo;

    /* AUTODOC: get default IPv4 packet */
    rc = prvTgfPacketIpv4PacketDefaultPacketGet(&packetInfo.numOfParts,
                                                &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");

    /* AUTODOC: get packet header part*/
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E,
                                   &packetInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    ipv4PartPtr->totalLen = PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS;

    /* AUTODOC: changed Ipv4 header part*/
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,0,
                                             ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &packetInfo,
                                   (GT_VOID *)&vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    prvTgfIpv4PacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfIpv4PacketInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfIpv4PacketInfo.partsArray = packetInfo.partsArray;
}


/**
* @internal tgfTunnelTcamCheckLookupOrderBasicRuleConfigurationSet function
* @endinternal
*
* @brief   Set basic rule configuration
*/
GT_VOID tgfTunnelTcamCheckLookupOrderBasicRuleConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ruleAction, 0, sizeof(ruleAction));

    /* AUTODOC: enable TTI lookup for port 0, key PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* configure basic TTI rule action */
    ruleAction.tunnelTerminate       = GT_FALSE;
    ruleAction.ttPassengerPacketType = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ruleAction.command               = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand       = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.tunnelStart           = GT_FALSE;
    ruleAction.tag0VlanCmd           = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;

    ruleAction.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    ruleAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ruleAction.tag0VlanId                      = 0x06;

    ruleAction.bridgeBypass     = GT_TRUE;
    ruleAction.continueToNextTtiLookup = GT_FALSE;
    ruleAction.modifyMacDa      = GT_TRUE;
}

/**
* @internal tgfTunnelTcamCheckLookupOrderEnableTti1Lookup function
* @endinternal
*
* @brief   Enable 2nd TTI lookup
*
* @param[in] continueToNextTtiLookup  - enables next TTI lookup
*                                       None
*/
GT_VOID tgfTunnelTcamCheckLookupOrderEnableTti1Lookup
(
    IN GT_BOOL continueToNextTtiLookup
)
{
    ruleAction.continueToNextTtiLookup = continueToNextTtiLookup;
}

/**
* @internal tgfTunnelTcamCheckLookupOrderConfigurationSet function
* @endinternal
*
* @brief   Configure TTI rules
*
* @param[in] numOfRules               - the number of rules to set
* @param[in] step                     - increment value to send less traffic on GM
*                                       None
*/
GT_VOID tgfTunnelTcamCheckLookupOrderConfigurationSet
(
    IN GT_U32 numOfRules,
    IN GT_U32 step
)
{
    GT_STATUS            rc                   = GT_OK;
    GT_U32               prvTgfRouterArpIndex = 0;
    GT_ETHERADDR         arpMacAddr =  {{0x77, 0x77, 0x77, 0x77, 0x00, 0x00}};
    PRV_TGF_TTI_RULE_UNT pattern;
    PRV_TGF_TTI_RULE_UNT mask;

    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask, 0, sizeof(mask));

    /* AUTODOC: set TCAM segment mode for IPv4 key */
    if (PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E,
                                         PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E);
    }
    else
    {
        rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E,
                                         PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");

    for (prvTgfRouterArpIndex = 0; prvTgfRouterArpIndex < numOfRules; prvTgfRouterArpIndex++)
    {
        ruleAction.arpPtr = prvTgfRouterArpIndex;

        arpMacAddr.arEther[4] = (GT_U8) (prvTgfRouterArpIndex >> 8);
        arpMacAddr.arEther[5] = (GT_U8) prvTgfRouterArpIndex;

        /* AUTODOC: write a ARP MAC 77:77:77:77:xx:xx to the Router ARP Table */
        rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* create invalid rules with each <step>-rule valid */
        if (prvTgfRouterArpIndex % step != 0)
        {
            pattern.udbArray[0] = 0xFE;
            mask.udbArray[0] = 0xFF;
        }
        else
        {
            pattern.udbArray[0] = 0x00;
            mask.udbArray[0] = 0x00;
        }

        /* set TTI rule */
        rc = prvTgfTtiRule2Set(prvTgfRouterArpIndex, PRV_TGF_TTI_KEY_IPV4_E,
                               &pattern, &mask, &ruleAction);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d",
                                     prvTgfDevNum);
    };
}

/**
* @internal tgfTunnelTcamCheckLookupOrderTrafficOnPort function
* @endinternal
*
* @brief   Check expected traffic
*
* @param[in] capturedPort             - port to check traffic on
*                                       None
*/
GT_VOID tgfTunnelTcamCheckLookupOrderTrafficOnPort
(
    GT_U32 capturedPort
)
{
    GT_STATUS               rc           = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[2];
    GT_U32                  numTriggers  = 0;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = capturedPort;

    /* Prepare VFD array entries for Identification field of IPv4 header */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 4;
    vfdArray[0].cycleCount = sizeof(GT_U8);
    vfdArray[0].modeExtraInfo=0;
    vfdArray[0].incValue=1;
    vfdArray[0].patternPtr[0]=(GT_U8) (currentStep >> 8);
    vfdArray[0].cycleCount=1;

    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 5;
    vfdArray[1].cycleCount = sizeof(GT_U8);
    vfdArray[1].modeExtraInfo=0;
    vfdArray[1].incValue=1;
    vfdArray[1].patternPtr[0]= (GT_U8) currentStep;
    vfdArray[1].cycleCount=1;

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2,
                                                        vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                               "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                               " port = %d, rc = 0x%02X\n",
                               portInterface.devPort.portNum, rc);

    /* check if captured packet has the same pattern as vfdArray */
    UTF_VERIFY_EQUAL1_STRING_MAC(3, numTriggers,
                    "Packet macDA is wrong, currentStep: [%d]", currentStep);
}

/**
* @internal tgfTunnelTcamCheckLookupOrderTrafficGenerate function
* @endinternal
*
* @brief   Generate Traffic
*/
GT_VOID tgfTunnelTcamCheckLookupOrderTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32    portIter    = 0;
    GT_U32    prvTgfBurstCount = 1;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC:   send 1  IPv4  packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfIpv4PacketInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check captured egress traffic on port */
    tgfTunnelTcamCheckLookupOrderTrafficOnPort(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    return;
}

/**
* @internal tgfTunnelTcamCheckLookupOrderStartTraffic function
* @endinternal
*
* @brief   Start transmitting packets
*
* @param[in] numOfRules               - number of packets to send
* @param[in] step                     - increment value to send less traffic on GM
*                                       None
*/
GT_VOID tgfTunnelTcamCheckLookupOrderStartTraffic
(
    IN GT_U32 numOfRules,
    IN GT_U32 step
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i  = 0;

    currentStep =
        (numOfRules == PRV_TGF_TTI1_SIZE_CNS) ? PRV_TGF_TTI0_SIZE_CNS : 0;

    tgfTunnelTcamCheckLookupOrderTrafficGenerate();

    /* AUTODOC: invalidate first (0 for TTI0 or 5120 for TTI1) TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(currentStep, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfTtiRuleValidStatusSet: %d",
                                  prvTgfDevNum);
    currentStep += step;

    /* send packets with step [128] */
    for (; currentStep < numOfRules; currentStep += step)
    {
        tgfTunnelTcamCheckLookupOrderTrafficGenerate();

        for (i = currentStep - step + 1; i <= currentStep; i++)
        {
            /* AUTODOC: invalidate TTI rules passed after step increment */
            rc = prvTgfTtiRuleValidStatusSet(i, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfTtiRuleValidStatusSet: %d",
                                          prvTgfDevNum);
        }
    }

    /* invalidate rules that left */
    if (currentStep > numOfRules)
    {
        for (i = currentStep - step + 1; i <= numOfRules; i++)
        {
            /* AUTODOC: invalidate TTI rule */
            rc = prvTgfTtiRuleValidStatusSet(i, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvTgfTtiRuleValidStatusSet: %d",
                                          prvTgfDevNum);
        }
    }
}

/**
* @internal tgfTunnelTcamCheckLookupOrderConfigurationRestore function
* @endinternal
*
* @brief   Restore base configuration
*/
GT_VOID tgfTunnelTcamCheckLookupOrderConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: RESTORE CONFIGURATION: */

    GT_STATUS          rc = GT_OK;
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d",
                                 prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d",
                                 prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for port 0, key PRV_TGF_TTI_KEY_IPV4_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}


