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
* @file prvTgfTunnelTermEtherOverMplsPwLabel.c
*
* @brief Tunnel term Ethernet over MPLS with PW label
*
* @version   7
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermIpv4overIpv4.h>
#include <common/tgfPacketGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS          6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* the TTI Rule index */
static GT_U32        prvTgfTtiRuleIndex        = 0;

/* the Mask for MAC address */
static TGF_MAC_ADDR  prvTgfFullMaskMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* Base CPU code */
static GT_U32 baseCpuCode;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_MAC_MODE_ENT      macMode;
    GT_U8                         udbOfsetArray[PRV_TGF_TTI_MAX_UDB_CNS];
    PRV_TGF_TTI_OFFSET_TYPE_ENT   udbOffsetTypeArray[PRV_TGF_TTI_MAX_UDB_CNS];
    CPSS_PACKET_CMD_ENT           illegalControlWordErrorCmd;
    GT_U32                        baseCpuCode;
    CPSS_PORT_SPEED_ENT           speed;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
} prvTgfRestoreCfg;

/******************************* Test packet **********************************/

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x05},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x15}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart = {
    0x1000,             /* label */
    0,                  /* CoS */
    1,                  /* S */
    0xff                /* TTL */
};

static GT_U8 prvTgfPacketPwLabelPartArr[] = {
    0x00, 0x00, 0x00, 0x00
};

/* MPLS PW Label part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPwLabelPart = {
    sizeof(prvTgfPacketPwLabelPartArr),                         /* dataLength */
    prvTgfPacketPwLabelPartArr                                  /* dataPtr */
};

static GT_U8 prvTgfPacketPwLabelWithIllegalControlWordPartArr[] = {
    0x20, 0x00, 0x00, 0x00
};

/* MPLS PW Label part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPwLabelWithIllegalControlWordPart = {
    sizeof(prvTgfPacketPwLabelWithIllegalControlWordPartArr),   /* dataLength */
    prvTgfPacketPwLabelWithIllegalControlWordPartArr            /* dataPtr */
};

/* packet's Ethernet over MPLS (start of passenger part) */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x12}                /* saMac */
};

/* VLAN_TAG passenger part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of basic packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPwLabelPart},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet with illegal control word */
static TGF_PACKET_PART_STC prvTgfPacketWithIllegalControlWordPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPwLabelWithIllegalControlWordPart},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, vid 5 */
    rc = prvTgfBrgDefFdbMacEntryOnVidSet(prvTgfPacketL2Part.daMac, PRV_TGF_SEND_VLANID_CNS,
                                         PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefFdbMacEntryOnVidSet: %d", prvTgfDevNum);
}

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
	
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    /* AUTODOC: set TTI Action */
    ttiAction.tunnelTerminate                   = GT_TRUE;
    ttiAction.ttPassengerPacketType             = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                   = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum    = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiAction.cwBasedPw                         = GT_TRUE;
    ttiAction.applyNonDataCwCommand             = GT_TRUE;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_MPLS_E */

    /* AUTODOC: enable the TTI lookup for MPLS at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: save the current lookup MAC mode for MPLS and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    /* AUTODOC: set lookup MAC mode for MPLS and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_MPLS_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: save the current base CPU code */
    rc = prvTgfTtiPwCwCpuCodeBaseGet(prvTgfDevNum, &prvTgfRestoreCfg.baseCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwCpuCodeBaseGet");
    baseCpuCode = prvTgfRestoreCfg.baseCpuCode;

    /* AUTODOC: save the current command for illegal control word */
    rc = prvTgfTtiPwCwExceptionCmdGet(prvTgfDevNum, PRV_TGF_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E, &prvTgfRestoreCfg.illegalControlWordErrorCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwExceptionCmdGet");

    /* AUTODOC: set TTI Pattern */
    cpssOsMemCpy(ttiPattern.mpls.common.mac.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    ttiPattern.mpls.common.pclId                = 0;
    ttiPattern.mpls.common.srcIsTrunk           = 0;
    ttiPattern.mpls.common.srcPortTrunk         = 0;
    ttiPattern.mpls.common.vid                  = PRV_TGF_SEND_VLANID_CNS;
    ttiPattern.mpls.common.isTagged             = GT_TRUE;
    ttiPattern.mpls.common.dsaSrcIsTrunk        = GT_FALSE;
    ttiPattern.mpls.common.dsaSrcPortTrunk      = 0;
    ttiPattern.mpls.common.dsaSrcDevice         = prvTgfDevNum;
    ttiPattern.mpls.common.sourcePortGroupId    = 0;
    ttiPattern.mpls.label0                      = prvTgfPacketMplsLabelPart.label;
    ttiPattern.mpls.exp0                        = prvTgfPacketMplsLabelPart.exp;
    ttiPattern.mpls.label1                      = 0;
    ttiPattern.mpls.exp1                        = 0;
    ttiPattern.mpls.label2                      = 0;
    ttiPattern.mpls.exp2                        = 0;
    ttiPattern.mpls.numOfLabels                 = 1;
    ttiPattern.mpls.protocolAboveMPLS           = 2;

    /* AUTODOC: set TTI Mask */
    cpssOsMemCpy(ttiMask.mpls.common.mac.arEther, prvTgfFullMaskMac, sizeof(TGF_MAC_ADDR));
    ttiMask.mpls.common.srcIsTrunk = 1;
    ttiMask.mpls.common.vid = 0xfff;
    ttiMask.mpls.label0 = 0xfffff;

    /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with action: */
    /* AUTODOC:   cmd FORWARD */
    /* AUTODOC:   MPLS passenger packet type */
    /* AUTODOC:   DA=00:00:00:00:34:02 */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_MPLS_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelExceptionCommandSet function
* @endinternal
*
* @brief   Set a PW CW exception command
*
* @param[in] exceptionType            - exception type
* @param[in] command                  - the packet command
*                                       None
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelExceptionCommandSet
(
    IN PRV_TGF_TTI_PW_CW_EXCEPTION_ENT  exceptionType,
    IN CPSS_PACKET_CMD_ENT              command
)
{
    GT_STATUS rc;

    rc = prvTgfTtiPwCwExceptionCmdSet(prvTgfDevNum, exceptionType ,command);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwExceptionCmdSet");
}

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelCpuCodeBaseSet function
* @endinternal
*
* @brief   Set the base CPU code value for PWE3
*
* @param[in] cpuCodeBase              - the base CPU code value for PWE3
*                                      (APPLICABLE RANGES: 192..255)
*                                       None
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelCpuCodeBaseSet
(
    IN GT_U32   cpuCodeBase
)
{
    GT_STATUS rc;

    rc = prvTgfTtiPwCwCpuCodeBaseSet(prvTgfDevNum, cpuCodeBase);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwCpuCodeBaseSet");

    baseCpuCode = cpuCodeBase;
}

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @note 1. Restore TTI Configuration
*       2. Restore Base Configuration
*
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc        = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore TTI Configuration
     */

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: restore the current base CPU code */
    rc = prvTgfTtiPwCwCpuCodeBaseSet(prvTgfDevNum, prvTgfRestoreCfg.baseCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwCpuCodeBaseSet");

    /* AUTODOC: restore the command for illegal control word */
    rc = prvTgfTtiPwCwExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_PW_CW_EXCEPTION_ILLEGAL_CONTROL_WORD_E, prvTgfRestoreCfg.illegalControlWordErrorCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPwCwExceptionCmdSet");
   
    /* AUTODOC: restore the lookup Mac mode for MPLS and DA */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_MPLS_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for MPLS at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    
    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore default Precedence on port 0 */
    rc = prvTgfBrgVlanPortVidPrecedenceSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidPrecedenceSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E);

    /* AUTODOC: invalidate VLAN entry 6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);

    /*Sets default values for global parametsers*/
    prvTgfPacketRestoreDefaultParameters();

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
}

/**
* @internal tgfTunnelTermEtherOverMplsPwLabelTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] packetType               -    0: send legal packet
*                                      1: send a packet with illegal control word
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
* @param[in] trapToCpu                -     whether to trap traffic to the CPU
*                                       None
*/
GT_VOID tgfTunnelTermEtherOverMplsPwLabelTrafficGenerate
(
    IN GT_U32   packetType,
    IN GT_BOOL  expectTraffic,
    IN GT_BOOL  trapToCpu
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          numOfExpectedRxPackets, numOfTrappedPackets;
    GT_BOOL                         getFirst;
    static GT_U8                    packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                          buffLen;
    GT_U32                          packetActualLength = 0;
    TGF_NET_DSA_STC                 rxParam;
    GT_U8                           dev = 0;
    GT_U8                           queue = 0;
    GT_U32                          cpuCode, expectedCpuCode;

    /* AUTODOC: GENERATE TRAFFIC: */

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    switch (packetType)
    {
        case 1:
            packetInfo.partsArray = prvTgfPacketWithIllegalControlWordPartArray;
            expectedCpuCode = baseCpuCode + 4;
            break;

        default:
            packetInfo.partsArray = prvTgfPacketPartArray;
            expectedCpuCode = baseCpuCode;
            break;
    }

    if (expectTraffic == GT_FALSE)
    {
        numOfExpectedRxPackets = 0;
    }
    else
    {
        numOfExpectedRxPackets = prvTgfBurstCount;
    }

    if (trapToCpu == GT_FALSE)
    {
        numOfTrappedPackets = 0;
    }
    else
    {
        numOfTrappedPackets = prvTgfBurstCount;
    }

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:05, SA=00:00:00:00:00:15 */
    /* AUTODOC:   VID=5, Ethertype=0x8847, MPLS Label=0x1000 */
    /* AUTODOC:   Passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:12 */

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get Ethernet packet on port 3 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:12 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

                /* AUTODOC: packetSize is not changed */
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

                break;

        case PRV_TGF_EGR_PORT_IDX_CNS:

                /* packetSize is changed. The new packet will be without VLAN_TAG and MPLS parts */
                expectedPacketSize = (packetSize - 2 * TGF_VLAN_TAG_SIZE_CNS - 2 * TGF_MPLS_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * numOfExpectedRxPackets;
                expectedCntrs.goodPktsSent.l[0]   = numOfExpectedRxPackets;
                expectedCntrs.ucPktsSent.l[0]     = numOfExpectedRxPackets;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * numOfExpectedRxPackets;
                expectedCntrs.goodPktsRcv.l[0]    = numOfExpectedRxPackets;
                expectedCntrs.ucPktsRcv.l[0]      = numOfExpectedRxPackets;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

            default:

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

                break;
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
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    getFirst = GT_TRUE;
    buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    for (; numOfTrappedPackets > 0; numOfTrappedPackets--)
    {
        /* AUTODOC: get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet\n");

        /* AUTODOC: check the CPU code */
        cpuCode = (GT_U32)rxParam.cpuCode;
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuCode, cpuCode - CPSS_NET_FIRST_USER_DEFINED_E + 192,
                                     "Wrong CPU code, expected: %d received %d",
                                     expectedCpuCode, cpuCode - CPSS_NET_FIRST_USER_DEFINED_E + 192);
        getFirst = GT_FALSE;
    }
    /* AUTODOC: get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet\n");
}


