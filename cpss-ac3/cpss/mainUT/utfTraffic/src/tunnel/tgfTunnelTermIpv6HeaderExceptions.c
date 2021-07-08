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
* @file tgfTunnelTermIpv6HeaderExceptions.c
*
* @brief Verify the functionality of Tunnel Term Ipv6 Header Exceptions
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
#include <common/tgfIpGen.h>
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
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* the TTI Rule indexes */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)

/* tti key type for the test */
static PRV_TGF_TTI_KEY_TYPE_ENT ttiKeyType = PRV_TGF_TTI_KEY_UDB_IPV6_E;

/* cpu code for the test */
static GT_U32 expectedCpuCode = CPSS_NET_IP_HDR_ERROR_E;

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/*  IPv6 Template */
static TGF_PACKET_STC packetInfo;

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader - no next header for IPv6         */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Exception info for restore */
static struct
{
    CPSS_PACKET_CMD_ENT      cmd;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
}exceptionInfo;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfTunnelTermIpv6HeaderExceptionsBridgeConfigSet function
* @endinternal
*
*/
static GT_VOID tgfTunnelTermIpv6HeaderExceptionsBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal tgfTunnelTermIpv6HeaderExceptionsTtiConfigSet function
* @endinternal
*
*/
static GT_VOID tgfTunnelTermIpv6HeaderExceptionsTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Action for matched packet (not l4 valid) */
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiAction.ingressPipeBypass = GT_TRUE;

    ttiAction.tag0VlanCmd       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId        = PRV_TGF_EGR_VLANID_CNS;

    /* AUTODOC: enable the TTI lookup for ttiKeyType at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                        ttiKeyType, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key ttiKeyType */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, ttiKeyType, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: add TTI rule 1 with ttiKeyType on port 0 VLAN 5 with action: redirect to egress */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, ttiKeyType,
                                       &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}


/**
* @internal tgfTunnelTermIpv6HeaderExceptionsCheckCpu function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID tgfTunnelTermIpv6HeaderExceptionsCheckCpu
(
    IN GT_BOOL  expectTraffic
)
{
    GT_STATUS           rc;
    GT_U32              numOfTrappedPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;
    GT_BOOL             getFirst = GT_TRUE;
    GT_U32              buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32              packetActualLength = 0;
    TGF_NET_DSA_STC     rxParam;
    GT_U8               queue = 0;
    GT_U32              cpuCode;
    GT_U8               packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U8               dev = 0;

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
        UTF_VERIFY_EQUAL2_STRING_MAC(expectedCpuCode, cpuCode,
                                     "Wrong CPU code, expected: %d received %d",
                                     expectedCpuCode, cpuCode);
        getFirst = GT_FALSE;
    }

    /* AUTODOC: get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet\n");
}


/**
* @internal tgfTunnelTermIpv6HeaderExceptionsTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID tgfTunnelTermIpv6HeaderExceptionsTrafficGenerate
(
    IN GT_BOOL  expectTraffic,
    IN GT_BOOL  expectTrapToCpu
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          numOfExpectedRxPackets;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

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

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled ipv6 packet from port 0 */
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

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        switch(portIter)
        {
           case PRV_TGF_SEND_PORT_IDX_CNS:
                numOfExpectedRxPackets = prvTgfBurstCount;
                break;
           case PRV_TGF_EGR_PORT_IDX_CNS:
                numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;
                break;
            default:
                numOfExpectedRxPackets = 0;
        }

        /* AUTODOC: verify counters */
        UTF_VERIFY_EQUAL3_STRING_MAC(numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0],
            "got wrong counters: port[%d], expected [%d], received[%d]\n",
             prvTgfPortsArray[portIter], numOfExpectedRxPackets, portCntrs.goodPktsRcv.l[0]);
    }

    /* AUTODOC: check trap to cpu traffic and cpu code */
    tgfTunnelTermIpv6HeaderExceptionsCheckCpu(expectTrapToCpu);
}

/**
* @internal tgfTunnelTermIpv6HeaderExceptionsConfigRestore function
* @endinternal
*
*/
static GT_VOID tgfTunnelTermIpv6HeaderExceptionsConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key ttiKeyType */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      ttiKeyType, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore original packet */
    cpssOsMemSet(prvTgfPacketIpv6Part.srcAddr, 0, sizeof(TGF_IPV6_ADDR));
    cpssOsMemSet(prvTgfPacketIpv6Part.dstAddr, 0, sizeof(TGF_IPV6_ADDR));
    prvTgfPacketIpv6Part.srcAddr[0] = 0x6545;
    prvTgfPacketIpv6Part.srcAddr[7] = 0x3212;
    prvTgfPacketIpv6Part.dstAddr[0] = 0x1122;
    prvTgfPacketIpv6Part.dstAddr[6] = 0xccdd;
    prvTgfPacketIpv6Part.dstAddr[7] = 0xeeff;
}

/**
* @internal tgfTunnelTermIpv6HeaderExceptionsSaveConfig function
* @endinternal
*
*/
static GT_VOID tgfTunnelTermIpv6HeaderExceptionsSaveConfig
(
    GT_BOOL                      isRestore,
    PRV_TGF_TTI_EXCEPTION_ENT    exception
)
{
    GT_STATUS           rc;

    if(GT_FALSE == isRestore)
    {
        /* AUTODOC: save ipv6 tti header error command (for configuration restore) */
        rc = prvTgfTtiExceptionCmdGet(prvTgfDevNum, exception, &exceptionInfo.cmd);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdGet");

        /* AUTODOC: Save exception CPU code */
        rc = prvTgfTtiExceptionCpuCodeGet(prvTgfDevNum, exception, &exceptionInfo.cpuCode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCpuCodeGet");
    }
    else
    {
        /* AUTODOC: restore ipv6 tti header error command (for configuration restore) */
        rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, exception, exceptionInfo.cmd);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

        /* AUTODOC: restore exception CPU code */
        rc = prvTgfTtiExceptionCpuCodeSet(prvTgfDevNum, exception, exceptionInfo.cpuCode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCpuCodeSet");
    }
}

/**
* @internal tgfTunnelTermIpv6HeaderExceptionsCheckOneException function
* @endinternal
*
* @brief   check one exception logic
*         save exception configuration
*         ------------------
*         set ipv6 tti header error command to CPSS_PACKET_CMD_FORWARD_E
*         send packet and expect ipv6 header exception
*         expect packet forward and no trap to cpu
*         ------------------
*         set ipv6 tti header error command to CPSS_PACKET_CMD_DROP_HARD_E
*         send packet and expect ipv6 header exception
*         expect packet drop and no trap to cpu
*         ------------------
*         set ipv6 tti header error command to CPSS_PACKET_CMD_TRAP_TO_CPU_E
*         set exception CPU code CPSS_NET_IP_HDR_ERROR_E
*         send packet and expect ipv6 header exception
*         expect packet trap to cpu with configured cpu code
*         ------------------
*         restore exception configuration
*/
static GT_VOID tgfTunnelTermIpv6HeaderExceptionsCheckOneException
(
    PRV_TGF_TTI_EXCEPTION_ENT exception
)
{
    GT_STATUS   rc;

    /* AUTODOC: save exception configuration  */
    tgfTunnelTermIpv6HeaderExceptionsSaveConfig(GT_FALSE, exception);

    /*------------------------------------------------------------*/
    PRV_UTF_LOG0_MAC("======= Check exception command CPSS_PACKET_CMD_DROP_HARD_E =======\n");

    /* AUTODOC: set ipv6 tti header error command to CPSS_PACKET_CMD_DROP_HARD_E */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, exception, CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    /* AUTODOC: send packet and expect ipv6 header exception */
    /* AUTODOC: expect packet drop, and no trap to cpu   */
    tgfTunnelTermIpv6HeaderExceptionsTrafficGenerate(GT_FALSE, GT_FALSE);

    /*------------------------------------------------------------*/
    if(GT_FALSE == prvUtfIsGmCompilation())
    {
        /* In BC2A in GM (only in GM!) packet command FWD cannot mask exception
               and action will not be performed in any case.
           This is a bug TTI-892 that was fixed in Verifier (GM) after BC2A tapeout
        */

        PRV_UTF_LOG0_MAC("======= Check exception command CPSS_PACKET_CMD_FORWARD_E =======\n");

        /* AUTODOC: set ipv6 tti header error command to CPSS_PACKET_CMD_FORWARD_E */
        rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, exception, CPSS_PACKET_CMD_FORWARD_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

        /* AUTODOC: send packet and expect ipv6 header exception */
        /* AUTODOC: expect packet forward, and no trap to cpu   */
        tgfTunnelTermIpv6HeaderExceptionsTrafficGenerate(GT_TRUE, GT_FALSE);
    }

    /*------------------------------------------------------------*/
    PRV_UTF_LOG0_MAC("======= Check exception command CPSS_PACKET_CMD_TRAP_TO_CPU_E =======\n");

    /* AUTODOC: set ipv6 tti header error command to CPSS_PACKET_CMD_TRAP_TO_CPU_E */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, exception, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    /* AUTODOC: set exception CPU code CPSS_NET_IP_HDR_ERROR_E */
    rc = prvTgfTtiExceptionCpuCodeSet(prvTgfDevNum, exception, expectedCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCpuCodeSet");

    /* AUTODOC: send packet and expect ipv6 header exception */
    /* AUTODOC: expect packet trap to cpu */
    tgfTunnelTermIpv6HeaderExceptionsTrafficGenerate(GT_FALSE, GT_TRUE);

    /*------------------------------------------------------------*/
    /* AUTODOC: restore exception configuration  */
    tgfTunnelTermIpv6HeaderExceptionsSaveConfig(GT_TRUE, exception);
}


/**
* @internal tgfTunnelTermIpv6HeaderExceptions function
* @endinternal
*
*
* @note 1) Exception check and exception command assignment performed before TTI lookup
*       (and works well in TTI).
*       2) Exception masking check logic performed before IP unit logic (and doesn't work in TTI)
*       for masking exceptions command FORWARD should be used for masking exceptions (see below)
*       3) Exception check command FORWARD allows to perform TTI action.
*       If exception detected, forward command applied and action performed.
*
*/
GT_VOID tgfTunnelTermIpv6HeaderExceptions
(
    GT_VOID
)
{
    /* AUTODOC: build correct IPv6 Packet  */
    packetInfo.numOfParts = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo.partsArray = prvTgfPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    tgfTunnelTermIpv6HeaderExceptionsBridgeConfigSet();

    /* AUTODOC: set the TTI Rule Pattern and Mask to match all packets */
    /* AUTODOC: add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with */
    /* AUTODOC: TTI action: modify vid to 6, bypass ingress, redirect to port 36 */
    tgfTunnelTermIpv6HeaderExceptionsTtiConfigSet();

    PRV_UTF_LOG0_MAC("\n\n==================================================================\n");
    PRV_UTF_LOG0_MAC("======= Check legal packet - no exception ==========\n");
    /* AUTODOC: send packet and expect TTI match, and traffic on port 36  */
    tgfTunnelTermIpv6HeaderExceptionsTrafficGenerate(GT_TRUE, GT_FALSE);

    PRV_UTF_LOG0_MAC("\n\n==================================================================\n");
    PRV_UTF_LOG0_MAC("======= Check ipv6 header error - wrong version =======\n");

    /* AUTODOC: modify packet, ipv6 header version to 7 (wrong version) */
    prvTgfPacketIpv6Part.version = 7;

    /* AUTODOC: check header version exception */
    tgfTunnelTermIpv6HeaderExceptionsCheckOneException(PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E);

    /* AUTODOC: restore ipv6 header version */
    prvTgfPacketIpv6Part.version = 6;

    PRV_UTF_LOG0_MAC("\n\n==================================================================\n");
    PRV_UTF_LOG0_MAC("======= Check ipv6 header error - SIP == DIP =======\n");

    /* AUTODOC: modify packet, set packet SIP = DIP */
    cpssOsMemCpy(prvTgfPacketIpv6Part.srcAddr, prvTgfPacketIpv6Part.dstAddr,
                 sizeof(prvTgfPacketIpv6Part.srcAddr));

    /* AUTODOC: check header SIP == DIP exception */
    tgfTunnelTermIpv6HeaderExceptionsCheckOneException(PRV_TGF_TTI_EXCEPTION_IPV6_HEADER_ERROR_E);

    PRV_UTF_LOG0_MAC("\n\n==================================================================\n");
    PRV_UTF_LOG0_MAC("======= Check ipv6 SIP exception - SIP is FF:: (multicast) =======\n");

    /* AUTODOC: modify packet, set packet SIP ff:: - multicast */
    prvTgfPacketIpv6Part.srcAddr[0] = 0xFF<<8;

    /* AUTODOC: check packet SIP exception */
    tgfTunnelTermIpv6HeaderExceptionsCheckOneException(PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E);

    PRV_UTF_LOG0_MAC("\n\n==================================================================\n");
    PRV_UTF_LOG0_MAC("======= Check ipv6 SIP exception - SIP is ::1 (loopback) =======\n");

    /* AUTODOC: modify packet, set packet SIP ::1 - loopback */
    cpssOsMemSet(prvTgfPacketIpv6Part.srcAddr, 0, sizeof(prvTgfPacketIpv6Part.srcAddr));
    prvTgfPacketIpv6Part.srcAddr[7] = 1;

    /* AUTODOC: check packet SIP exception */
    tgfTunnelTermIpv6HeaderExceptionsCheckOneException(PRV_TGF_TTI_EXCEPTION_IPV6_SIP_ADDRESS_ERROR_E);

    /*------------------------------------------------------------*/
    /* AUTODOC: test configuration restore */
    tgfTunnelTermIpv6HeaderExceptionsConfigRestore();
}


