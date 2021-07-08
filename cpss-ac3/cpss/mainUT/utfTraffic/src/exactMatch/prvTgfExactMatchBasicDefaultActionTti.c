/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfExactMatchBasicDefaultActionTti.c
*
* @brief Configuration for Exact Match
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfCommon.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionTti.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>


/******************************* Test packet **********************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS             1

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR1_PORT_IDX_CNS       1
#define PRV_TGF_EGR2_PORT_IDX_CNS       2
#define PRV_TGF_EGR3_PORT_IDX_CNS       3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

#define PRV_TGF_CPU_PORT_IDX_CNS        3

/* profileId number to set */
#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS 10

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 2

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS       1

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 1;



/* parameters that is needed to be restored */
static struct
{
    GT_U32                              sendPort;
    GT_BOOL                             profileIdEn;
    GT_U32                              profileId;
    GT_BOOL                             defaultActionEn;
    PRV_TGF_EXACT_MATCH_ACTION_UNT      defaultAction;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT      lookupClientType;

} prvTgfRestoreCfg;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x44, 0x33, 0x22, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,         /* totalLen   */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};
/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/****************************************************************************************************************************************************/
/****************************************************************************************************************************************************/

/*Reset the port counters */
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfExactMatchBasicDefaultActionTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
* @note Used 4 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiGenericConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    prvTgfRestoreCfg.sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = 16;
    }
    /* init configuration */
    prvTgfTunnelTermEtherTypeOtherTestInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_TRUE, 0);
}

/**
* @internal prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiGenericConfigRestore
(
    GT_VOID
)
{
    prvTgfTunnelTermEtherTypeOtherConfigRestore();

    /* AUTODOC: restore send port */
    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = prvTgfRestoreCfg.sendPort;
}
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Exact Match Default Action
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_TTI_KEY_TYPE_ENT                keyType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    GT_BOOL                                 enableExactMatchLookup;
    GT_BOOL                                 enableDefaultAction;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          action ;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT          clientType;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &action, 0, sizeof(action));

    keyType=PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;
    ttiLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    enableExactMatchLookup = GT_TRUE ;

    action.ttiAction.command = PRV_TGF_PACKET_CMD_DROP_HARD_E ;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
    enableDefaultAction = GT_TRUE ;

    /*save */
    rc = prvTgfExactMatchTtiProfileIdGet(prvTgfDevNum, keyType, ttiLookupNum , &prvTgfRestoreCfg.profileIdEn , &prvTgfRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExactMatchProfileIdGet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchProfileDefaultActionGet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS ,
                                                 actionType,&prvTgfRestoreCfg.defaultAction,&prvTgfRestoreCfg.defaultActionEn);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionGet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum,ttiLookupNum,&prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /*set test */
    clientType = PRV_TGF_EXACT_MATCH_CLIENT_TTI_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,ttiLookupNum, clientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchTtiProfileIdSet(prvTgfDevNum, keyType, ttiLookupNum , enableExactMatchLookup , PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExactMatchProfileIdSet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,actionType,&action,enableDefaultAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);

}
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Exact Match
*          Default Action
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_TTI_KEY_TYPE_ENT                keyType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          zeroAction;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));

    keyType = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;
    ttiLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    /* AUTODOC: Restore first lookup client type */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum,ttiLookupNum, prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* AUTODOC: restore Exact Match profileId */
    rc= prvTgfExactMatchTtiProfileIdSet(prvTgfDevNum,
                                        keyType,
                                        ttiLookupNum,
                                        prvTgfRestoreCfg.profileIdEn,
                                        prvTgfRestoreCfg.profileId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiExactMatchProfileIdSet FAILED: %d", prvTgfDevNum);

    if (prvTgfRestoreCfg.defaultActionEn == GT_TRUE )
    {
        /* AUTODOC: restore Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                     actionType,
                                                     &prvTgfRestoreCfg.defaultAction,
                                                     GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: restore Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                                     actionType,
                                                     &zeroAction,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }
}
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiCommandTrap function
* @endinternal
*
* @brief   set default action command to be trap with cpu code [502]
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiCommandTrap
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_BOOL                                 enableDefaultAction;

    IN PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT  actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          action ;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &action, 0, sizeof(action));

    action.ttiAction.command = PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E ;
    action.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
    enableDefaultAction = GT_TRUE ;

    rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,actionType,&action,enableDefaultAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
}
/**
* @internal prvTgfExactMatchBasicDefaultActionTtiRuleValidStatusSet function
* @endinternal
*
* @brief   This function validates / invalidates the rule in TCAM
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiRuleValidStatusSet
(
    GT_BOOL state
)
{
    GT_STATUS   rc;
    /* invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, state);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, state);
}

/**
* @internal prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
* @param[in] expectTrapTraffic   - GT_FALSE: expect no traffic
*                                - GT_TRUE: expect traffic to be trapped
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
*
*/
GT_VOID prvTgfExactMatchBasicDefaultActionTtiTrafficGenerate
(
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic prvTgfTunnelTermEtherTypeTrafficGenerateExpectTraffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    PRV_UTF_LOG0_MAC("======= set capture on all ports =======\n");
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* enable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* enable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* enable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC:   DA=00:00:44:33:22:11, SA=00:00:00:00:00:33, VID=1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* ======= disable capture on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    /* disable capture on port 3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    /* disable capture on port 2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS]);

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    /* disable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get no traffic - dropped */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter) {
            case PRV_TGF_SEND_PORT_IDX_CNS:

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

                break;

        default:
             if (expectForwardTraffic == GT_TRUE)
             {
                /* port has both Rx and Tx counters because of capture */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
             }
             else
             {
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]    = 0;
                expectedCntrs.ucPktsSent.l[0]      = 0;
                expectedCntrs.brdcPktsSent.l[0]    = 0;
                expectedCntrs.mcPktsSent.l[0]      = 0;
                expectedCntrs.goodOctetsRcv.l[0]   = 0;
                expectedCntrs.goodPktsRcv.l[0]     = 0;
                expectedCntrs.ucPktsRcv.l[0]       = 0;
                expectedCntrs.brdcPktsRcv.l[0]     = 0;
                expectedCntrs.mcPktsRcv.l[0]       = 0;
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

            PRV_UTF_LOG0_MAC("Current values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", portCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", portCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", portCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", portCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", portCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", portCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", portCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", portCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", portCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", portCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* get trigger counters */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* ======= check on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    if (expectForwardTraffic==GT_TRUE)
    {
        /* check if there are captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers, "the test expected traffic to be forwarded\n");
    }
    else
    {
        /* check if no captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, numTriggers, "the test expected traffic NOT to be forwarded\n");

        if (GT_TRUE == prvTgfActionTrapToCpuCheck[PRV_TGF_CPU_PORT_IDX_CNS])
        {
            GT_U8                           packetBufPtr[64] = {0};
            GT_U32                          packetBufLen = 64;
            GT_U32                          packetLen;
            GT_U8                           devNum;
            GT_U8                           queueCpu;
            TGF_NET_DSA_STC                 rxParams;

            PRV_UTF_LOG0_MAC("CPU port capturing:\n");

            cpssOsBzero((GT_VOID*)&rxParams, sizeof(TGF_NET_DSA_STC));
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE,
                                               packetBufPtr,
                                               &packetBufLen,
                                               &packetLen,
                                               &devNum,
                                               &queueCpu,
                                               &rxParams);

            if(expectTrapTraffic==GT_FALSE)
            {
                /* we want to verify no packet was trapped to CPU
                   the command in the Exact Match Default Action was DROP */
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS,
                                                rxParams.cpuCode);

                PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                PRV_UTF_LOG0_MAC("Packet is NOT Trapped to  CPU. Packet is dropped\n\n");
            }
            else
            {
                 /* we want to verify packet was trapped to CPU */
                 UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                 UTF_VERIFY_EQUAL0_PARAM_MAC(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS,rxParams.cpuCode);

                 PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                 PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n\n",rxParams.cpuCode);
            }
        }
    }

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

