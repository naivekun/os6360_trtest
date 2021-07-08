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
* @file prvTgfTunnelTermEVlanPrecDualLookup.c
*
* @brief Tunnel: Test Dual Lookup Policy Based Routing Pointer
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermEVlanPrecDualLookup.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS            6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* nextHop port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS     3

/* Number of ports in send VLan */
#define PRV_TGF_RX_SIDE_PORTS_COUNT_CNS 2

/* Total Number of ports */
#define PRV_TGF_TOTAL_PORTS_COUNT_CNS 4

/* the IPCL rule index */
#define PCL_RULE_INDEX_ON_FLOOR_0_CNS    1

/* the TTI Rule indexes */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 , 3)
#define PRV_TGF_TTI1_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1 , 0)

static PRV_TGF_TTI_ACTION_2_STC ttiAction = {0};
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;

static struct
{
    GT_BOOL lookupEnable;
} prvTgfRestoreCfg;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthernetVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                          /* pri, cfi, VlanId */
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

/* PARTS of packet Ethernet */
static TGF_PACKET_PART_STC prvTgfPacketEthernetPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketEthernetVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET Ethernet to send */
static TGF_PACKET_STC prvTgfPacketEthernetInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketEthernetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthernetPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermEVlanPrecDualLookupBridgeConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*/

static GT_VOID prvTgfTunnelTermEVlanPrecDualLookupBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc   = GT_FAIL;
    GT_U8 tagArray[] = {1,1,1,1};

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);


    /* AUTODOC: add FDB entry with daMac, vlanId, send port */
     rc = prvTgfBrgDefFdbMacEntryOnPortSet(
         prvTgfPacket1L2Part.daMac, PRV_TGF_SEND_VLANID_CNS,
         prvTgfDevNum,
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
         GT_TRUE);

     UTF_VERIFY_EQUAL1_STRING_MAC(
         GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with daMac, vlanId, receive port */
     rc = prvTgfBrgDefFdbMacEntryOnPortSet(
         prvTgfPacket1L2Part.daMac, PRV_TGF_EGR_VLANID_CNS,
         prvTgfDevNum,
         prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
         GT_TRUE);

     UTF_VERIFY_EQUAL1_STRING_MAC(
         GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelTermEVlanPrecDualLookupTtiConfigurationSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
static GT_VOID prvTgfTunnelTermEVlanPrecDualLookupTtiConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");


    /* AUTODOC: Store the TTI lookup enabling status on send port for
                PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E,
                                      &prvTgfRestoreCfg.lookupEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    /* AUTODOC: enable the TTI lookup for ETH at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action to match all packets */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    ttiAction.tag0VlanCmd       =  PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanId         = PRV_TGF_SEND_VLANID_CNS;
    ttiAction.tag0VlanPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.continueToNextTtiLookup   = GT_TRUE;

    /* AUTODOC: set TTI0 rule with vlan precedence SOFT */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* AUTODOC: set TTI1 rule with vlan precedence SOFT */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI1_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermEVlanPrecDualLookupPclConfigurationSet function
* @endinternal
*
* @brief   Set IPCL Configuration
*/
static GT_VOID prvTgfTunnelTermEVlanPrecDualLookupPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc       = GT_FAIL;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      pclAction;

    PRV_UTF_LOG0_MAC("======= Setting PCL Configuration =======\n");

    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&pclAction, 0, sizeof(pclAction));

    pclAction.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
    pclAction.vlan.vlanId     = PRV_TGF_EGR_VLANID_CNS;

    /* AUTODOC: Set the IPCL rule */
    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            PCL_RULE_INDEX_ON_FLOOR_0_CNS, &mask, &pattern, &pclAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 PCL_RULE_INDEX_ON_FLOOR_0_CNS);
}


/**
* @internal prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence function
* @endinternal
*
* @brief   Change TTI Vlan Precedence and Vlan command for given TTI rule index
*
* @param[in] precedence               - set Vlan Precedence
* @param[in] vlanCmd                  - set Vlan Cmd
* @param[in] ruleIndex                - index of TTI rule
*                                       None
*/
static GT_VOID prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence
(
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT precedence,
    PRV_TGF_TTI_VLAN_COMMAND_ENT                vlanCmd,
    GT_U32                                      ruleIndex
)
{
    GT_STATUS rc = GT_FAIL;

    ttiAction.tag0VlanPrecedence = precedence;
    ttiAction.tag0VlanCmd = vlanCmd;

    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
}



/**
* @internal prvTgfTunnelTermEVlanPrecDualLookupTrafficOnPort function
* @endinternal
*
* @brief   Checks packet on egress port
*/
static GT_VOID prvTgfTunnelTermEVlanPrecDualLookupTrafficOnPort
(
    GT_U32 capturedPortNum
)
{
    GT_STATUS rc          = GT_FAIL;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];
    GT_U32                  numTriggers  = 0;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = capturedPortNum;

    /* Prepare VFD array entries for VLAN tag */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 15;
    vfdArray[0].modeExtraInfo=0;
    vfdArray[0].incValue=1;
    vfdArray[0].patternPtr[0] = 0x06;
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
* @internal prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE : expect traffic
*                                      GT_FALSE: expect no traffic
*                                       None
*/
static GT_VOID prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate
(
    GT_BOOL expectTraffic
)
{
    GT_STATUS rc               = GT_FAIL;
    GT_U32    prvTgfBurstCount = 1;
    GT_U32    numOfExpectedRxPackets    = 1;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32    portIter;

    /* AUTODOC: GENERATE TRAFFIC */

    /*  Reset all Ethernet port's counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 1 ethernet packet from port 0 with: VID=5 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    &prvTgfPacketEthernetInfo, prvTgfBurstCount,
                                    0, NULL, prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    numOfExpectedRxPackets = expectTraffic ? prvTgfBurstCount : 0;

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        if(PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* AUTODOC: verify to get packet on port 0 with: */
            UTF_VERIFY_EQUAL3_STRING_MAC(portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                "got wrong counters: port[%d], expected [%d], received[%d]\n",
                 prvTgfPortsArray[portIter], prvTgfBurstCount, portCntrs.goodPktsRcv.l[0]);
        }
        else
        if(PRV_TGF_EGR_PORT_IDX_CNS == portIter)
        {
            /* AUTODOC: verify to get packet on port 3 with: */
            UTF_VERIFY_EQUAL3_STRING_MAC(portCntrs.goodPktsRcv.l[0], numOfExpectedRxPackets,
                "got wrong counters: port[%d], expected [%d], received[%d]\n",
                 prvTgfPortsArray[portIter], numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0]);

            if (numOfExpectedRxPackets)
            {
                prvTgfTunnelTermEVlanPrecDualLookupTrafficOnPort(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);
            }
        }
        else
        {
            /* AUTODOC: verify to get no packet on port: */
            UTF_VERIFY_EQUAL3_STRING_MAC(portCntrs.goodPktsRcv.l[0], 0,
                "got wrong counters: port[%d], expected [%d], received[%d]\n",
                 prvTgfPortsArray[portIter], 0, portCntrs.goodPktsRcv.l[0]);
        }
    }
}

/**
* @internal prvTgfTunnelTermEVlanPrecDualLookupConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfTunnelTermEVlanPrecDualLookupConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_FAIL;
    GT_U32      i  = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* AUTODOC: invalidate TTI rule 0 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PCL_RULE_INDEX_ON_FLOOR_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PCL_RULE_INDEX_ON_FLOOR_0_CNS,
                                 GT_FALSE);

    /* AUTODOC: disable the TTI lookup for ETH at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.lookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: Remove VLAN 5 members */
    for (i = 0; i < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: Remove VLAN 6 members */
    for (i = PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i < PRV_TGF_TOTAL_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}


/**
* @internal prvTgfTunnelTermEVlanPrecDualLookup function
* @endinternal
*
* @brief   Test main func
*/
GT_VOID prvTgfTunnelTermEVlanPrecDualLookup
(
    GT_VOID
)
{
    /* AUTODOC: build ethernet packet with vid 5 */

    /* AUTODOC: create VLAN 5 with tagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with tagged ports [2,3] */
    prvTgfTunnelTermEVlanPrecDualLookupBridgeConfigSet();
    /*------------------------------------------------------------*/

    /*
        AUTODOC: CASE 1:
        AUTODOC: set TTI0 rule with vlan precedence SOFT
        AUTODOC: set TTI1 rule with vlan precedence SOFT
    */
    prvTgfTunnelTermEVlanPrecDualLookupTtiConfigurationSet();

    /* AUTODOC: set PCL rule with modify vlan to '0x06' */
    prvTgfTunnelTermEVlanPrecDualLookupPclConfigurationSet();

    /* AUTODOC: send packet, expect packet vlan changed by IPCL to '0x06' and a packet on port */
    PRV_UTF_LOG0_MAC("======= CASE 1: TTI0 Precedence SOFT, TTI1 Precedence SOFT =======\n"
                     "======= CASE 1: TTI0 VlanCmd DO_NOT_MODIFY, TTI1 VlanCmd DO_NOT_MODIFY =======\n");
    prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_TRUE);
    /*------------------------------------------------------------*/

    /*
        AUTODOC: CASE 2:
        AUTODOC: set TTI0 rule vlan cmd to MODIFY
        AUTODOC: set TTI1 rule vlan cmd to MODIFY
        AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port
    */
    PRV_UTF_LOG0_MAC("======= CASE 2: TTI0 Precedence SOFT, TTI1 Precedence SOFT =======\n"
                     "======= CASE 2: TTI0 VlanCmd MODIFY, TTI1 VlanCmd MODIFY =======\n");
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E, PRV_TGF_TTI_VLAN_MODIFY_ALL_E, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E, PRV_TGF_TTI_VLAN_MODIFY_ALL_E, PRV_TGF_TTI1_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_TRUE);
    /*------------------------------------------------------------*/

    /*
        AUTODOC: CASE 3:
        AUTODOC: set TTI0 rule vlan precedence to HARD and Vlan Cmd to DO_NOT_MODIFY
        AUTODOC: set TTI1 rule Vlan Cmd to DO_NOT_MODIFY
        AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port
    */
    PRV_UTF_LOG0_MAC("======= CASE 3: TTI0 Precedence HARD, TTI1 Precedence SOFT =======\n"
                     "======= CASE 3: TTI0 VlanCmd DO_NOT_MODIFY, TTI1 VlanCmd DO_NOT_MODIFY =======\n");
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E, PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E, PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_TRUE);
    /*------------------------------------------------------------*/

    /*
        AUTODOC: CASE 4:
        AUTODOC: set TTI0 rule vlan precedence to SOFT
        AUTODOC: set TTI1 rule vlan precedence to HARD
        AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port
    */
    PRV_UTF_LOG0_MAC("======= CASE 4: TTI0 Precedence SOFT, TTI1 Precedence HARD =======\n"
                     "======= CASE 4: TTI0 VlanCmd DO_NOT_MODIFY, TTI1 VlanCmd DO_NOT_MODIFY =======\n");
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E, PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E, PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E, PRV_TGF_TTI1_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_TRUE);
    /*------------------------------------------------------------*/

    /*
        AUTODOC: CASE 5:
        AUTODOC: set TTI0 rule vlan precedence to HARD and Vlan Cmd to MODIFY_ALL
        AUTODOC: set TTI1 rule vlan precedence to SOFT and Vlan Cmd to MODIFY_ALL
        AUTODOC: send packet, expect packet vlan not changed by IPCL and no traffic on port
    */
    PRV_UTF_LOG0_MAC("======= CASE 5: TTI0 Precedence HARD, TTI1 Precedence SOFT =======\n"
                     "======= CASE 5: TTI0 VlanCmd MODIFY, TTI1 VlanCmd MODIFY =======\n");
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E, PRV_TGF_TTI_VLAN_MODIFY_ALL_E, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E, PRV_TGF_TTI_VLAN_MODIFY_ALL_E, PRV_TGF_TTI1_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_FALSE);
    /*------------------------------------------------------------*/

    /*
        AUTODOC: CASE 6:
        AUTODOC: set TTI0 rule vlan precedence to SOFT
        AUTODOC: set TTI1 rule vlan precedence to HARD
        AUTODOC: send packet, expect packet vlan not changed by IPCL and no traffic on port
    */
    PRV_UTF_LOG0_MAC("======= CASE 6: TTI0 Precedence SOFT, TTI1 Precedence HARD =======\n"
                     "======= CASE 6: TTI0 VlanCmd MODIFY, TTI1 VlanCmd MODIFY =======\n");
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E, PRV_TGF_TTI_VLAN_MODIFY_ALL_E, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E, PRV_TGF_TTI_VLAN_MODIFY_ALL_E, PRV_TGF_TTI1_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_FALSE);
    /*------------------------------------------------------------*/

    /*
        AUTODOC: CASE 7: Dual Lookup for Vlan cmd
        AUTODOC: set TTI0 rule vlan precedence to SOFT with Vlan MODIFY
        AUTODOC: set TTI1 rule vlan precedence to HARD with vlan DO_NOT_MODIFY
    */
    PRV_UTF_LOG0_MAC("======= CASE 7: Dual Lookup for Vlan cmd =======\n"
                     "======= CASE 7: TTI0 Precedence SOFT, TTI1 Precedence HARD =======\n"
                     "======= CASE 7: TTI0 VlanCmd MODIFY, TTI1 VlanCmd DO_NOT_MODIFY =======\n");
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E, PRV_TGF_TTI_VLAN_MODIFY_ALL_E, PRV_TGF_TTI0_INDEX_CNS);
    prvTgfTunnelTermEVlanPrecDualLookupSetPrecedence(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E, PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E, PRV_TGF_TTI1_INDEX_CNS);
    if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: send packet, expect packet vlan not changed by IPCL and traffic on port */
        prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_FALSE);
    }
    else
    {
        /* FE: TTI-1114 */
        /* AUTODOC: send packet, expect packet vlan changed by IPCL and traffic on port */
        prvTgfTunnelTermEVlanPrecDualLookupTrafficGenerate(GT_TRUE);
    }

    /*------------------------------------------------------------*/

    /* AUTODOC: test configuration restore */
    prvTgfTunnelTermEVlanPrecDualLookupConfigurationRestore();
}



