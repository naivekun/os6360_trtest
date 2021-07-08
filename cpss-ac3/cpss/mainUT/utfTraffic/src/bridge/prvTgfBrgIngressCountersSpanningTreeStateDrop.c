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
* @file prvTgfBrgIngressCountersSpanningTreeStateDrop.c
*
* @brief Verify that packets that are dropped due to Spanning Tree state
* drop are counted in field<LocalPropDisc> in counter-set0 and
* counter-set1 on different modes
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgIngressCountersSpanningTreeStateDrop.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* VLAN Id */
#define PRV_TGF_VLANID_CNS                      2

/* STG Id */
#define PRV_TGF_SIP_5_STGID_CNS                 333
#define PRV_TGF_STGID_CNS                       1
static GT_U16   tempStg = PRV_TGF_STGID_CNS;

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            3

/* first port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_1_CNS             0

/* second port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_2_CNS             1

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            2

/* default number of packets to send */
static GT_U32  prvTgfBurstCountArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    3, 5
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},
    {0, 1, 0, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},
    {0, 1, 0, 0}
};

/* expected number of capturing packets */
static GT_U8 numTriggersArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    0, 0
};

/* array of DA macs for test */
static TGF_MAC_ADDR prvTgfBrgDaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}
};

/* array of SA macs for test */
static TGF_MAC_ADDR prvTgfBrgSaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x44}
};

/* counter set number for test */
static PRV_TGF_BRG_CNTR_SET_ID_ENT prvTgfBrgCounterSetNumberArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_BRG_CNTR_SET_ID_0_E,
    PRV_TGF_BRG_CNTR_SET_ID_1_E
};

/* expected number Bridge ingress counter */
static GT_U32 prvTgfBridgeIngresssCountrArr[PRV_TGF_SEND_PACKETS_NUM_CNS][4] =
{
    {8, 0, 0, 8},
    {3, 0, 0, 3}
};

/* array of send portIdx for test */
static  GT_U8 prvTgfBrgSendPortArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_SEND_PORT_IDX_1_CNS,
    PRV_TGF_SEND_PORT_IDX_2_CNS
};

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/******************************************************************************/
/*************************** Restore config ***********************************/
/* Parameters needed to be restored */

static struct
{
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode1;
    GT_PORT_NUM                         port1;
    GT_U16                              vlan1;
    CPSS_BRIDGE_INGR_CNTR_MODES_ENT     setMode2;
    GT_PORT_NUM                         port2;
    GT_U16                              vlan2;
    CPSS_STP_STATE_ENT                  state1;
    CPSS_STP_STATE_ENT                  state2;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgIngressCountersSpanningTreeStateDropConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgIngressCountersSpanningTreeStateDropConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    tempStg = PRV_TGF_STGID_CNS;
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        tempStg = PRV_TGF_SIP_5_STGID_CNS;
    }

    /* AUTODOC: bind VLAN 2 to STG Id 1 */
    rc = prvTgfBrgVlanStgIdBind(PRV_TGF_VLANID_CNS, tempStg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanStgIdBind: %d", prvTgfDevNum);

    /* save STP state of portIdx 0 */
    rc = prvTgfBrgStpPortStateGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                  tempStg, &prvTgfRestoreCfg.state1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateGet: %d", prvTgfDevNum);

    /* save STP state of portIdx 1 */
    rc = prvTgfBrgStpPortStateGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                  tempStg, &prvTgfRestoreCfg.state2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateGet: %d", prvTgfDevNum);

    /* AUTODOC: set STP state to learning for port 0 */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                  tempStg, CPSS_STP_LRN_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d", prvTgfDevNum);

    /* AUTODOC: set STP state to blocking for port 1 */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                  tempStg, CPSS_STP_BLCK_LSTN_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d", prvTgfDevNum);

    /* save bridge ingress counters for counter set 0*/
    rc = prvTgfBrgCntBridgeIngressCntrModeGet(prvTgfDevNum, PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                              &prvTgfRestoreCfg.setMode1, &prvTgfRestoreCfg.port1,
                                              &prvTgfRestoreCfg.vlan1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntBridgeIngressCntrModeGet: %d", prvTgfDevNum);

    /* save bridge ingress counters for counter set 1*/
    rc = prvTgfBrgCntBridgeIngressCntrModeGet(prvTgfDevNum, PRV_TGF_BRG_CNTR_SET_ID_1_E,
                                              &prvTgfRestoreCfg.setMode2, &prvTgfRestoreCfg.port2,
                                              &prvTgfRestoreCfg.vlan2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntBridgeIngressCntrModeGet: %d", prvTgfDevNum);

    /* AUTODOC: config for cntSet0 Bridge Ingress cnt mode CNT_MODE_0 */
    rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                              CPSS_BRG_CNT_MODE_0_E,
                                              0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntBridgeIngressCntrModeSet: %d", prvTgfDevNum);

    /* AUTODOC: config for cntSet1 Bridge Ingress cnt mode CNT_MODE_1, port 0 */
    rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_1_E,
                                              CPSS_BRG_CNT_MODE_1_E,
                                              prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS], 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntBridgeIngressCntrModeSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgIngressCountersSpanningTreeStateDropTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgIngressCountersSpanningTreeStateDropTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U8                           portIter = 0;
    GT_U8                           sendIter = 0;
    GT_U32                          packetLen = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_BRIDGE_INGRESS_CNTR_STC    ingressCntr = {0};


    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCommonAllCntrsReset");

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* set Dst Mac */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfBrgDaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* set Srs Mac */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfBrgSaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                 prvTgfBurstCountArr[sendIter], 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: send 3 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:11, VID=2 */
        /* AUTODOC: send 5 packets on port 1 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:44, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfBrgSendPortArr[sendIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfBrgSendPortArr[sendIter]]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic is dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            packetLen = (portIter == prvTgfBrgSendPortArr[sendIter]) ?
                prvTgfTaggedPacketInfo.totalLen :
                prvTgfTaggedPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS;

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        packetLen, prvTgfBurstCountArr[sendIter]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

      /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* set vfd for destination MAC */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfBrgDaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        UTF_VERIFY_EQUAL2_STRING_MAC(numTriggersArr[sendIter], numTriggers,
                         "get another trigger that expected: expected - %d, recieved - %d\n",
                         numTriggersArr[sendIter], numTriggers);
    }

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* clear current counters value */
        cpssOsMemSet((GT_VOID*) &ingressCntr, 0, sizeof(ingressCntr));

        /* AUTODOC: get Bridge ingress counters: */
        rc = prvTgfBrgCntBridgeIngressCntrsGet(prvTgfDevNum, prvTgfBrgCounterSetNumberArr[sendIter],
                                               &ingressCntr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgCntBridgeIngressCntrsGet");

        /* AUTODOC:   check Bridge ingress counters - gtBrgInFrames */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][0],
                                     ingressCntr.gtBrgInFrames,
                   "get another gtHostInPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][0], ingressCntr.gtBrgInFrames);

        /* AUTODOC:   check Bridge ingress counters - gtBrgVlanIngFilterDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][1],
                                     ingressCntr.gtBrgVlanIngFilterDisc,
                   "get another gtHostOutBroadcastPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][1], ingressCntr.gtBrgVlanIngFilterDisc);

        /* AUTODOC:   check Bridge ingress counters - gtBrgSecFilterDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][2],
                                     ingressCntr.gtBrgSecFilterDisc,
                   "get another gtHostOutMulticastPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][2], ingressCntr.gtBrgSecFilterDisc);

        /* AUTODOC:   check Bridge ingress counters - gtBrgLocalPropDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBridgeIngresssCountrArr[sendIter][3],
                                     ingressCntr.gtBrgLocalPropDisc,
                   "get another gtHostOutPkts that expected: expected - %d, recieved - %d\n",
                   prvTgfBridgeIngresssCountrArr[sendIter][3], ingressCntr.gtBrgLocalPropDisc);
    }
}

/**
* @internal prvTgfBrgIngressCountersSpanningTreeStateDropConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgIngressCountersSpanningTreeStateDropConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCommonAllCntrsReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore STP state for port 0 */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                  tempStg, prvTgfRestoreCfg.state1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d", prvTgfDevNum);

    /* AUTODOC: restore STP state for port 1 */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                  tempStg, prvTgfRestoreCfg.state2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d", prvTgfDevNum);

    /* AUTODOC: restore bridge ingress counters for counter set 0 */
    rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_0_E,
                                              prvTgfRestoreCfg.setMode1, prvTgfRestoreCfg.port1,
                                              prvTgfRestoreCfg.vlan1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntBridgeIngressCntrModeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore bridge ingress counters for counter set 1 */
    rc = prvTgfBrgCntBridgeIngressCntrModeSet(PRV_TGF_BRG_CNTR_SET_ID_1_E,
                                              prvTgfRestoreCfg.setMode2, prvTgfRestoreCfg.port2,
                                              prvTgfRestoreCfg.vlan2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntBridgeIngressCntrModeSet: %d", prvTgfDevNum);
}


