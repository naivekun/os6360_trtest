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
* @file prvTgfBrgArpTrapping.c
*
* @brief ARP trapping configuration test
*
* @version   3
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <bridge/prvTgfBrgArpTrapping.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                5

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/****************************** ARP packet ************************************/

/* ethertype part of ARP packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketArpEtherTypePart = {
    TGF_ETHERTYPE_0806_ARP_TAG_CNS
};
/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacketArpPart = {
    0x01,                                   /* HW Type */
    0x0800,                                 /* Protocol (IPv4= 0x0800) */
    0x06,                                   /* HW Len = 6 */
    0x04,                                   /* Proto Len = 4 */
    0x01,                                   /* Opcode */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},   /* HW Address of Sender (MAC SA) */
    {0x11, 0x12, 0x13, 0x14},               /* Protocol Address of Sender(SIP)*/
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},   /* HW Address of Target (MAC DA) */
    {0x30, 0x31, 0x32, 0x33}                /* Protocol Address of Target(DIP)*/
};
/* PARTS of packet ARP */
static TGF_PACKET_PART_STC prvTgfPacketArpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketArpEtherTypePart},
    {TGF_PACKET_PART_ARP_E,       &prvTgfPacketArpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* LENGTH of packet ARP */
#define PRV_TGF_PACKET_ARP_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_ARP_HEADER_SIZE_CNS + sizeof(prvTgfPacket1PayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketArpInfo = {
    PRV_TGF_PACKET_ARP_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacketArpPartArray)/sizeof(TGF_PACKET_PART_STC),/* numOfParts*/
    prvTgfPacketArpPartArray                                     /* partsArray*/
};

/******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* stored default Vlan ID and ARP commands */
static GT_U16   prvTgfDefVlanId = 0;
static CPSS_PACKET_CMD_ENT saveTrappingCmd;


/**
* @internal prvTgfArpTrappingBrgConfigurationSet function
* @endinternal
*
* @brief   Set test bridge configuration
*/
GT_VOID prvTgfArpTrappingBrgConfigurationSet
(
    IN GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0,0,0,0};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                               "ERROR of prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: Save trapping to CPU ARP Broadcast packets for all VLANs */
    rc = prvTgfBrgGenArpBcastToCpuCmdGet(PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                         &saveTrappingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgGenArpBcastToCpuCmdGet: %d");

}
/**
* @internal prvTgfArpTrappingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*         INPUTS:
*         ipCntrlType - IP Control Type value in VLAN entry
*         cmdMode - command interface mode:
*         PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E - will set ARP broadcast command for all ports,
* @param[in] ipCntrlType              - IP Control Type value in VLAN entry
* @param[in] cmdMode                  - command interface mode:
*                                      PRV_TGF_BRG_ARP_BCAST_CMD_MODE_PORT_E - will set ARP broadcast command for all ports,
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*                                      PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E - will set ARP broadcast command for all vlans,
* @param[in] trappingCmd              - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
* @param[in] vlanNonIpv4BcCmd         - Non IPv4 Broadcast command in VLAN entry.
* @param[in] vlanIpv4BcCmd            - IPv4 Broadcast command in VLAN entry.
*                                       None
*/
GT_VOID prvTgfArpTrappingConfigurationSet
(
    IN PRV_TGF_BRG_IP_CTRL_TYPE_ENT       ipCntrlType,
    IN PRV_TGF_BRG_ARP_BCAST_CMD_MODE_ENT cmdMode,
    IN CPSS_PACKET_CMD_ENT  trappingCmd,
    IN CPSS_PACKET_CMD_ENT  vlanNonIpv4BcCmd,
    IN CPSS_PACKET_CMD_ENT  vlanIpv4BcCmd
)
{
    GT_STATUS   rc = GT_OK;

    rc = cpssDxChBrgVlanUnkUnregFilterSet(prvTgfDevNum, PRV_TGF_VLANID_CNS,
                                          CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,
                                          vlanNonIpv4BcCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChBrgVlanUnkUnregFilterSet: %d");

    rc = cpssDxChBrgVlanUnkUnregFilterSet(prvTgfDevNum, PRV_TGF_VLANID_CNS,
                                          CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E,
                                          vlanIpv4BcCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChBrgVlanUnkUnregFilterSet: %d");


    /* AUTODOC: enable  IP control traffic trapping to CPU */
    rc = prvTgfBrgVlanIpCntlToCpuSet(PRV_TGF_VLANID_CNS,
                                     ipCntrlType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpCntlToCpuSet: %d");

    /* AUTODOC: enable trapping to CPU ARP Broadcast packets for all VLANs */
    rc = prvTgfBrgGenArpBcastToCpuCmdSet(cmdMode, trappingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgGenArpBcastToCpuCmdSet: %d");

}

/**
* @internal prvTgfArpTrappingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] packetToCpu              - packet expectet on CPU:
* @param[in] floodOrDrop              - GT_TRUE - expected flooding
*                                      GT_FALSE - expected drop
* @param[in] expectCpuCode            - expected CPU code. Used when packetToCpu == TRUE.
*                                       None
*/
GT_VOID prvTgfArpTrappingTrafficGenerate
(
    IN GT_BOOL  packetToCpu,
    IN GT_BOOL  floodOrDrop,
    IN CPSS_NET_RX_CPU_CODE_ENT    expectCpuCode
)
{
    GT_STATUS                       rc       = GT_OK;
    GT_U32                          portIter = 0;

    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;

    GT_STATUS   expectedRcRxToCpu = GT_OK;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC:   send BC packet with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:11 */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketArpInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
                                 prvTgfDevNum);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            /*Tx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : ((floodOrDrop == GT_TRUE) ? 1 : 0)),
            prvTgfPacketArpInfo.totalLen,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if ( GT_FALSE == packetToCpu )
    {
        expectedRcRxToCpu = GT_NO_MORE;
    }

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");

    if (packetToCpu)
    {
        PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
        /* check CPU code */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectCpuCode, rxParam.cpuCode, "Wrong CPU Code");
    }

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");
}

/**
* @internal prvTgfArpTrappingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfArpTrappingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */


    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: restore trapping to CPU ARP Broadcast packets for all VLANs */
    rc = prvTgfBrgGenArpBcastToCpuCmdSet(PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                         saveTrappingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgGenArpBcastToCpuCmdSet: %d");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfArpTrappingTest function
* @endinternal
*
* @brief   ARP trapping configuration test
*/
GT_VOID prvTgfArpTrappingTest
(
    GT_VOID
)
{
    GT_U32 caseIdx = 1;

    /* Set Bridge configuration */
    prvTgfArpTrappingBrgConfigurationSet();

    PRV_UTF_LOG3_MAC("Case %d. VLAN IP Ctrl Mode %s,   VLAN ARP CMD %s\n", caseIdx++, "IPv4", "FORWARD");
    PRV_UTF_LOG2_MAC("        Non IPv4 BC CMD %s, IPv4 BC CMD %s\n\n", "FORWARD", "FORWARD");

    /* Set configuration */
    prvTgfArpTrappingConfigurationSet(PRV_TGF_BRG_IP_CTRL_IPV4_E, PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                      CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfArpTrappingTrafficGenerate(GT_FALSE, GT_TRUE, CPSS_NET_ALL_CPU_OPCODES_E /* not used*/);

    PRV_UTF_LOG3_MAC("Case %d. VLAN IP Ctrl Mode %s,   VLAN ARP CMD %s\n", caseIdx++, "IPv4", "MIRROR_TO_CPU");
    PRV_UTF_LOG2_MAC("        Non IPv4 BC CMD %s, IPv4 BC CMD %s\n\n", "FORWARD", "FORWARD");

    /* Set configuration */
    prvTgfArpTrappingConfigurationSet(PRV_TGF_BRG_IP_CTRL_IPV4_E, PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E, CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfArpTrappingTrafficGenerate(GT_TRUE, GT_TRUE, CPSS_NET_INTERVENTION_ARP_E);

    PRV_UTF_LOG3_MAC("Case %d. VLAN IP Ctrl Mode %s,   VLAN ARP CMD %s\n", caseIdx++, "IPv4", "TRAP_TO_CPU");
    PRV_UTF_LOG2_MAC("        Non IPv4 BC CMD %s, IPv4 BC CMD %s\n\n", "FORWARD", "FORWARD");

    /* Set configuration */
    prvTgfArpTrappingConfigurationSet(PRV_TGF_BRG_IP_CTRL_IPV4_E, PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E, CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfArpTrappingTrafficGenerate(GT_TRUE, GT_FALSE, CPSS_NET_INTERVENTION_ARP_E);

    PRV_UTF_LOG3_MAC("Case %d. VLAN IP Ctrl Mode %s,   VLAN ARP CMD %s\n", caseIdx++, "IPv4", "FORWARD");
    PRV_UTF_LOG2_MAC("        Non IPv4 BC CMD %s, IPv4 BC CMD %s\n\n", "MIRROR_TO_CPU", "FORWARD");

    /* Set configuration */
    prvTgfArpTrappingConfigurationSet(PRV_TGF_BRG_IP_CTRL_IPV4_E, PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                      CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfArpTrappingTrafficGenerate(GT_TRUE, GT_TRUE, CPSS_NET_NON_IPV4_BROADCAST_PACKET_E);

    PRV_UTF_LOG3_MAC("Case %d. VLAN IP Ctrl Mode %s,   VLAN ARP CMD %s\n", caseIdx++, "IPv4", "MIRROR_TO_CPU");
    PRV_UTF_LOG2_MAC("        Non IPv4 BC CMD %s, IPv4 BC CMD %s\n\n", "MIRROR_TO_CPU", "FORWARD");

    /* Set configuration */
    prvTgfArpTrappingConfigurationSet(PRV_TGF_BRG_IP_CTRL_IPV4_E, PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfArpTrappingTrafficGenerate(GT_TRUE, GT_TRUE, CPSS_NET_INTERVENTION_ARP_E);

    PRV_UTF_LOG3_MAC("Case %d. VLAN IP Ctrl Mode %s,   VLAN ARP CMD %s\n", caseIdx++, "None", "FORWARD");
    PRV_UTF_LOG2_MAC("        Non IPv4 BC CMD %s, IPv4 BC CMD %s\n\n", "MIRROR_TO_CPU", "FORWARD");

    /* Set configuration */
    prvTgfArpTrappingConfigurationSet(PRV_TGF_BRG_IP_CTRL_NONE_E, PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                      CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E, CPSS_PACKET_CMD_FORWARD_E);

    /* Generate traffic */
    prvTgfArpTrappingTrafficGenerate(GT_TRUE, GT_TRUE, CPSS_NET_NON_IPV4_BROADCAST_PACKET_E);

    PRV_UTF_LOG3_MAC("Case %d. VLAN IP Ctrl Mode %s,   VLAN ARP CMD %s\n", caseIdx++, "None", "FORWARD");
    PRV_UTF_LOG2_MAC("        Non IPv4 BC CMD %s, IPv4 BC CMD %s\n\n", "FORWARD", "MIRROR_TO_CPU");

    /* Set configuration */
    prvTgfArpTrappingConfigurationSet(PRV_TGF_BRG_IP_CTRL_NONE_E, PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                      CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E);

    /* Generate traffic */
    prvTgfArpTrappingTrafficGenerate(GT_FALSE, GT_TRUE, CPSS_NET_ALL_CPU_OPCODES_E /* not used*/);

    /* Restore configuration */
    prvTgfArpTrappingConfigurationRestore();
}

