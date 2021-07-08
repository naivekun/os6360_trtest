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
* @file prvTgfBrgEgressCountersUnknownPacketsTcDp.c
*
* @brief Verify that unknown packets are received
* in <outMcFrames> field according the
* configured Tc/ DP
*
* @version   5
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
#include <common/tgfMirror.h>
#include <common/tgfPortGen.h>
#include <common/tgfCosGen.h>
#include <bridge/prvTgfBrgEgressCountersUnknownPacketsTcDp.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* VLAN Id */
#define PRV_TGF_VLANID_CNS                      2

/* QOS index */
#define PRV_TGF_SA_QOS_INDEX_CNS                1

/* number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* number to capture traffic to */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            1

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            2

/* number of packets to send */
#define PRV_TGF_CONF_ENTRY_NUM_CNS              2

/* default number of packets to send */
static GT_U32  prvTgfBurstCountArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    3, 5
};

/* Counter Set ID array */
static GT_U8  prvTgfCntSetIdArr[] =
{
    0, 1
};

/* egress counters mode bitmap array */
static CPSS_PORT_EGRESS_CNT_MODE_ENT  prvTgfCntModeArr[] =
{
    CPSS_EGRESS_CNT_TC_E, CPSS_EGRESS_CNT_DP_E
};

/* drop precedence levels array */
static CPSS_DP_LEVEL_ENT  prvTgfDropPrLevelArr[] =
{
    CPSS_DP_GREEN_E, CPSS_DP_RED_E
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},
    {1, 0, 0, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 1, 1, 1},
    {1, 1, 1, 1}
};

typedef GT_U32 egresssCountrArr_type[PRV_TGF_SEND_PACKETS_NUM_CNS][7];

/* expected number  */
static egresssCountrArr_type prvTgfBridgeEgresssCountrArr =
{
    {0, 0, 0, 0, 0, 15, 0},
    {0, 0, 0, 0, 0, 9, 0}
};
/* the TXQ of the Puma3 count also packets that go to the fabric ports */
static egresssCountrArr_type prvTgfBridgeEgresssCountrArr_Puma3 =
{
    {0, 0, 0, 0, 0, 15 + 5, 0},
    {0, 0, 0, 0, 0, 9  + 3, 0}
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
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}
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
    3, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
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
    GT_U16                                  vlanId;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgEgressCountersUnknownPacketsTcDpConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgEgressCountersUnknownPacketsTcDpConfigSet
(
    GT_VOID
)
{
    GT_STATUS                              rc = GT_OK;
    GT_U8                                  tagArray[]  = {1, 1, 1, 1};
    GT_U8                                  confIter = 0;
    PRV_TGF_BRG_MAC_ENTRY_STC              macEntry;
    PRV_TGF_QOS_ENTRY_STC                  macQosCfg;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* save default Vlan ID on port 0 */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: create VLAN 2 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
                                           prvTgfPortsArray, NULL,
                                           tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: set PVID 2 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);

    /* AUTODOC: enable assignment of a default QoS profile */
    rc = prvTgfCosUnkUcEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCosUnkUcEnableSet");

    /* clear entry */
    cpssOsMemSet(&macQosCfg, 0, sizeof(macQosCfg));

    macQosCfg.qosParams.dp = CPSS_DP_RED_E;
    macQosCfg.qosParams.dscp = 0;
    macQosCfg.qosParams.tc = 5;
    macQosCfg.qosParams.up = 0;
    macQosCfg.qosParamsModify.modifyDp = GT_TRUE;
    macQosCfg.qosParamsModify.modifyDscp = GT_FALSE;
    macQosCfg.qosParamsModify.modifyTc = GT_TRUE;
    macQosCfg.qosParamsModify.modifyUp = GT_FALSE;
    macQosCfg.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    /* AUTODOC: set QoS table entry with index 1 and: */
    /* AUTODOC:   enable modifing DP\TC */
    /* AUTODOC:   DP=DP_RED, TC=5, Soft precedence */
    rc = prvTgfCosMacQosEntrySet(PRV_TGF_SA_QOS_INDEX_CNS, &macQosCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCosMacQosEntrySet");

    /* AUTODOC: set QoS Marking Conflict Mode to SA lookup */
    rc = prvTgfCosMacQosConflictResolutionSet(PRV_TGF_QOS_MAC_RESOLVE_USE_SA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCosMacQosConflictResolutionSet");

    /* clear entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* fill mac entry by defaults for this task */
    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.dstInterface.index           = 1;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[0];
    macEntry.isStatic                     = GT_TRUE;
    macEntry.saQosIndex                   = PRV_TGF_SA_QOS_INDEX_CNS;
    macEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_CNS;

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfBrgSaMacArr[0], sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:11, VLAN 2, port 0, saQosIndex 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

    for (confIter = 0; confIter <PRV_TGF_CONF_ENTRY_NUM_CNS; confIter++)
    {
        /* AUTODOC: set Egress Counters on TC 0 and DP RED */
        rc = prvTgfPortEgressCntrModeSet(prvTgfCntSetIdArr[confIter], prvTgfCntModeArr[confIter],
                                         prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                         0, 0, prvTgfDropPrLevelArr[confIter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortEgressCntrModeSet");
    }
}

/**
* @internal prvTgfBrgEgressCountersUnknownPacketsTcDpTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgEgressCountersUnknownPacketsTcDpTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U8                           portIter = 0;
    GT_U8                           sendIter = 0;
    CPSS_PORT_EGRESS_CNTR_STC       egrCntrPtr = {0};
    egresssCountrArr_type           *egresssCountrArrPtr;

    /* AUTODOC: GENERATE TRAFFIC: */
    egresssCountrArrPtr = (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_PUMA3_E) ?
            &prvTgfBridgeEgresssCountrArr:
            &prvTgfBridgeEgresssCountrArr_Puma3;

    /* AUTODOC: clear egress counters */
    for (sendIter = 0; sendIter < PRV_TGF_CONF_ENTRY_NUM_CNS; sendIter++)
    {
        /* get egress counters to clear them */
        rc = prvTgfPortEgressCntrsGet(prvTgfCntSetIdArr[sendIter], &egrCntrPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortEgressCntrsGet");
    }

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* set Dst Mac */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfBrgDaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* set Srs Mac */
        cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfBrgSaMacArr[sendIter], sizeof(TGF_MAC_ADDR));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                 prvTgfBurstCountArr[sendIter], 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: send 3 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:11, VID=2, PRI=3 */
        /* AUTODOC: send 5 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:22, VID=2, PRI=3 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   3 packets - dropped */
        /* AUTODOC:   5 packets - received on ports 1,2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        prvTgfTaggedPacketInfo.totalLen,
                                        prvTgfBurstCountArr[sendIter]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }
    }

    for (sendIter = 0; sendIter < PRV_TGF_CONF_ENTRY_NUM_CNS; sendIter++)
    {
        /* clear current counters value */
        cpssOsMemSet((GT_VOID*) &egrCntrPtr, 0, sizeof(egrCntrPtr));

        /* AUTODOC: get egress counters */
        rc = prvTgfPortEgressCntrsGet(prvTgfCntSetIdArr[sendIter], &egrCntrPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortEgressCntrsGet");

        /* AUTODOC:   check egress counters - brgEgrFilterDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC((*egresssCountrArrPtr)[sendIter][0],
                                     egrCntrPtr.brgEgrFilterDisc,
            "get another brgEgrFilterDisc that expected: expected - %d, recieved - %d\n",
            (*egresssCountrArrPtr)[sendIter][0], egrCntrPtr.brgEgrFilterDisc);

        /* AUTODOC:   check egress counters - egrFrwDropFrames */
        UTF_VERIFY_EQUAL2_STRING_MAC((*egresssCountrArrPtr)[sendIter][1],
                                     egrCntrPtr.egrFrwDropFrames,
            "get another egrFrwDropFrames that expected: expected - %d, recieved - %d\n",
            (*egresssCountrArrPtr)[sendIter][1], egrCntrPtr.egrFrwDropFrames);

        /* AUTODOC:   check egress counters - outBcFrames */
        UTF_VERIFY_EQUAL2_STRING_MAC((*egresssCountrArrPtr)[sendIter][2],
                                     egrCntrPtr.outBcFrames,
            "get another outBcFrames that expected: expected - %d, recieved - %d\n",
            (*egresssCountrArrPtr)[sendIter][2], egrCntrPtr.outBcFrames);

        /* AUTODOC:   check egress counters - outCtrlFrames */
        UTF_VERIFY_EQUAL2_STRING_MAC((*egresssCountrArrPtr)[sendIter][3],
                                     egrCntrPtr.outCtrlFrames,
            "get another outCtrlFrames that expected: expected - %d, recieved - %d\n",
            (*egresssCountrArrPtr)[sendIter][3], egrCntrPtr.outCtrlFrames);

        /* AUTODOC:   check egress counters - outMcFrames */
        UTF_VERIFY_EQUAL2_STRING_MAC((*egresssCountrArrPtr)[sendIter][4],
                                     egrCntrPtr.outMcFrames,
            "get another outMcFrames that expected: expected - %d, recieved - %d\n",
            (*egresssCountrArrPtr)[sendIter][4], egrCntrPtr.outMcFrames);

        /* AUTODOC:   check egress counters - outUcFrames */
        UTF_VERIFY_EQUAL2_STRING_MAC((*egresssCountrArrPtr)[sendIter][5],
                                     egrCntrPtr.outUcFrames,
            "get another outUcFrames that expected: expected - %d, recieved - %d\n",
            (*egresssCountrArrPtr)[sendIter][5], egrCntrPtr.outUcFrames);

        /* AUTODOC:   check egress counters - txqFilterDisc */
        UTF_VERIFY_EQUAL2_STRING_MAC((*egresssCountrArrPtr)[sendIter][6],
                                     egrCntrPtr.txqFilterDisc,
            "get another txqFilterDisc that expected: expected - %d, recieved - %d\n",
            (*egresssCountrArrPtr)[sendIter][6], egrCntrPtr.txqFilterDisc);
    }
}

/**
* @internal prvTgfBrgEgressCountersUnknownPacketsTcDpConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgEgressCountersUnknownPacketsTcDpConfigRestore
(
    GT_VOID
)
{
    GT_U8                   confIter = 0;
    GT_STATUS               rc = GT_OK;
    PRV_TGF_QOS_ENTRY_STC   macQosCfg;


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

    /* AUTODOC: restore default Vlan ID on port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 prvTgfRestoreCfg.vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear entry */
    cpssOsMemSet(&macQosCfg, 0, sizeof(macQosCfg));

    /* AUTODOC: restore default QoS entry 1 */
    rc = prvTgfCosMacQosEntrySet(PRV_TGF_SA_QOS_INDEX_CNS, &macQosCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssExMxPmQosUnkUcEnableSet");

    /* AUTODOC: restore assignment of a default QoS profile */
    rc = prvTgfCosUnkUcEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCosUnkUcEnableSet");

    /* AUTODOC: restore Configure QoS Marking Conflict Mode */
    rc = prvTgfCosMacQosConflictResolutionSet(PRV_TGF_QOS_MAC_RESOLVE_USE_SA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCosMacQosConflictResolutionSet");

    for (confIter = 0; confIter <PRV_TGF_CONF_ENTRY_NUM_CNS; confIter++)
    {
        /* AUTODOC: restore Egress Counters mode */
        rc = prvTgfPortEgressCntrModeSet(prvTgfCntSetIdArr[confIter], 0,
                                         prvTgfPortsArray[0],
                                         0, 0, CPSS_DP_GREEN_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortEgressCntrModeSet");
    }
}


