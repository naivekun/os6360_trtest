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
* @file prvTgfTunnelTermEtherOverIpv4RedirectToEgressEport.c
*
* @brief Tunnel term Ethernet over Ipv4 redirect to egress ePort
*
* @version   8
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermEtherOverIpv4RedirectToEgressEport.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* default VLAN Id */
#define PRV_TGF_VLANID_6_CNS            6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        1

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   (UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)  - 5)

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           2

/* number of vfd's */
#define PRV_TGF_VFD_NUM_CNS             3

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 3;

/* expected number of packets on ports */
static GT_U8 prvTgfPacketsCountRxTxArr[PRV_TGF_PORTS_NUM_CNS] = { 1, 1, 0, 0 };

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/****************************** Test packet (ipv4) *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},   /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}    /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS              /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3e,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xff,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/***************** Incapsulation ethernet frame **********************/
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
    0, 0, PRV_TGF_VLANID_5_CNS              /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartInt = {0x0640};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    /* incapsulation ethernet frame part */
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartInt},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPartInt},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartInt},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS * 2  + \
    TGF_VLAN_TAG_SIZE_CNS  * 2  + \
    TGF_ETHERTYPE_SIZE_CNS * 2  + \
    TGF_IPV4_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                  /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
    CPSS_INTERFACE_INFO_STC       physicalInfo;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, vid 5 */
    rc = prvTgfBrgDefFdbMacEntryOnVidSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_5_CNS,
                                         PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test settings:
*         - Enable port 0 for Eth lookup
*         - Set Eth key lookup MAC mode to Mac DA
*         - Set TTI rule action
*         - Set TTI rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_MAC_MODE_ENT    macMode = PRV_TGF_TTI_MAC_MODE_DA_E;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_STC      ruleAction = {0};
    CPSS_INTERFACE_INFO_STC     physicalInfo;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &pattern, 0, sizeof(pattern));
    cpssOsMemSet((GT_VOID*) &mask,    0, sizeof(mask));

    /* AUTODOC: enable the TTI lookup for TTI_KEY_IPV4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* save the current lookup Mac mode for IPv4 and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_IPV4_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_IPV4 */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_TRUE;
    ruleAction.passengerPacketType   = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.tsPassengerPacketType = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    ruleAction.copyTtlFromTunnelHeader = GT_FALSE;

    ruleAction.mplsCmd = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;

    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    ruleAction.interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    ruleAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.interfaceInfo.devPort.portNum = PRV_TGF_DESTINATION_EPORT_CNS;

    ruleAction.tunnelStart    = GT_FALSE;
    ruleAction.tunnelStartPtr = 0;

    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId  = 0;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;

    ruleAction.modifyUpEnable = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;

    ruleAction.userDefinedCpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP3_E;

    /* configure TTI rule */
    pattern.ipv4.common.pclId    = 1;
    pattern.ipv4.common.srcIsTrunk = GT_FALSE;
    pattern.ipv4.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    cpssOsMemCpy((GT_VOID*)pattern.ipv4.common.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
                 sizeof(pattern.ipv4.common.mac));
    pattern.ipv4.common.vid      = PRV_TGF_VLANID_5_CNS;
    pattern.ipv4.common.isTagged = GT_TRUE;
    pattern.ipv4.tunneltype = 7;
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

    /* AUTODOC: add TTI rule 2 with: */
        /* AUTODOC:   key IPv4, cmd FORWARD, redirectCmd REDIRECT_TO_EGRESS */
        /* AUTODOC:   pattern pclId=1, srcPort=0, vid=5, tunneltype=7 */
        /* AUTODOC:   pattern MAC=00:00:00:00:34:02 */
        /* AUTODOC:   pattern srcIp=10.10.10.10, dstIp=4.4.4.4 */
        /* AUTODOC:   action Tunnel Terminate, PASSENGER_ETHERNET_NO_CRC, port=1 */
        /* AUTODOC:   tag0VlanCmd=DO_NOT_MODIFY, vid=6, tag1VlanCmd=MODIFY_UNTAGGED */
        /* AUTODOC:   tag1UpCommand=ASSIGN_VLAN1_UNTAGGED, modifyUpEnable=MODIFY_UP_ENABLE */
        /* AUTODOC:   userDefinedCpuCode=UDP_BC_MIRROR_TRAP3_E */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_IPV4_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

     /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* set ePort mapping configuration */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: map port 1 to ePort 1000 */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);
}

/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portIter    = 0;
    GT_U32          packetSize  = 0;
    GT_U32          packetIter  = 0;
    GT_U32          numTriggers = 0;
    TGF_VFD_INFO_STC         vfdArray[PRV_TGF_VFD_NUM_CNS];
    CPSS_INTERFACE_INFO_STC  portInterface;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* reset counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 3 Ethernet over IPv4 tunneled packets from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
        /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
        /* AUTODOC:   passenger DA=00:00:00:00:00:22, SA=00:00:00:00:00:25 */
        /* AUTODOC:   passenger VID=5, EtherType=0x0640 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get 3 Ethernet packets on port 1 (ePort 1000) with: */
        /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:25 */
        /* AUTODOC:   EtherType=0x0640 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* calculate packet length */
        packetSize = prvTgfPacketInfo.totalLen -
                     (TGF_L2_HEADER_SIZE_CNS  +
                      TGF_VLAN_TAG_SIZE_CNS * 2 +  /* untagged vlans used */
                      TGF_ETHERTYPE_SIZE_CNS  +
                      TGF_IPV4_HEADER_SIZE_CNS) * (portIter == PRV_TGF_EGR_PORT_IDX_CNS);

        /* check ETH counters */
        rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                    prvTgfPortsArray[portIter],
                                    prvTgfPacketsCountRxTxArr[portIter],
                                    prvTgfPacketsCountRxTxArr[portIter],
                                    packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* get trigger counters */

    /* check da mac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset     = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2PartInt.daMac, sizeof(TGF_MAC_ADDR));

    /* check sa mac */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset     = sizeof(TGF_MAC_ADDR);
    vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfPacketL2PartInt.saMac, sizeof(TGF_MAC_ADDR));

    /* check ethertype */
    vfdArray[2].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[2].modeExtraInfo = 0;
    vfdArray[2].offset     = sizeof(TGF_MAC_ADDR)*2;
    vfdArray[2].cycleCount = TGF_ETHERTYPE_SIZE_CNS;

    vfdArray[2].patternPtr[0] = (GT_U8)((prvTgfPacketEtherTypePartInt.etherType >> 8) & 0xFF);
    vfdArray[2].patternPtr[1] = (GT_U8)(prvTgfPacketEtherTypePartInt.etherType & 0xFF);

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                PRV_TGF_VFD_NUM_CNS, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                    portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    for(packetIter = 0; packetIter < prvTgfBurstCount; packetIter++)
    {
        /* check if captured packet has all correct vfd's */
        UTF_VERIFY_EQUAL1_STRING_MAC((1 << PRV_TGF_VFD_NUM_CNS) - 1,
                      numTriggers & ((1 << PRV_TGF_VFD_NUM_CNS) - 1),
            "   Errors while triggers summary result checking: packetIter =  %d\n", packetIter);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, numTriggers & BIT_0,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                        prvTgfPacketL2PartInt.daMac[0],
                                        prvTgfPacketL2PartInt.daMac[1],
                                        prvTgfPacketL2PartInt.daMac[2],
                                        prvTgfPacketL2PartInt.daMac[3],
                                        prvTgfPacketL2PartInt.daMac[4],
                                        prvTgfPacketL2PartInt.daMac[5]);

        /* check if captured packet has the same MAC SA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(GT_TRUE, (numTriggers & BIT_1) >> 1,
                "\n   MAC SA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                                        prvTgfPacketL2PartInt.saMac[0],
                                        prvTgfPacketL2PartInt.saMac[1],
                                        prvTgfPacketL2PartInt.saMac[2],
                                        prvTgfPacketL2PartInt.saMac[3],
                                        prvTgfPacketL2PartInt.saMac[4],
                                        prvTgfPacketL2PartInt.saMac[5]);

        /* check if captured packet has the same Ethertype as prvTgfArpMac */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, (numTriggers & BIT_2) >> 2,
                "\n   Ethertype of captured packet must be: %04X",
                                       prvTgfPacketEtherTypePartInt.etherType);

        /* prepare next iteration */
        numTriggers = numTriggers >> PRV_TGF_VFD_NUM_CNS;
    }
}

/**
* @internal prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTunnelTermEtherOverIpv4RedirectToEgressEportConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: invalidate TTI rule 2 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC_MODE_DA for TTI_KEY_IPV4 */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_IPV4_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup for TTI_KEY_IPV4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore ePort 1000 mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

