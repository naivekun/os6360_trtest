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
* @file prvTgfBrgEgressCountersCtrlToAnlyzrPortVlan.c
*
* @brief The test checks that packets are received
* in counter set 0, field <outCtrlFrames> in port modes
* in counter set 1, field <outCtrlFrames> in VLAN modes
*
* @version   6
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
#include <bridge/prvTgfBrgEgressCountersCtrlToAnlyzrPortVlan.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN ID */
#define PRV_TGF_DEF_VLANID_CNS                  1

/* VLAN Id */
#define PRV_TGF_VLANID_CNS                      2

/* STG Id */
#define PRV_TGF_STGID_CNS                       1

/* first port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* second port number for UC bridging */
#define PRV_TGF_FDB_PORT_IDX_CNS           1

/* third port number for analyzer */
#define PRV_TGF_ANALYZER_PORT_IDX_CNS           2

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            2

/* namber of packets to send */
#define PRV_TGF_CONF_ENTRY_NUM_CNS              2

/* namber of  */
#define PRV_TGF_EGRESS_CNC_NUM_CNS              7

/* interface index */
static GT_U32   prvTgfInterfaceIndex = 0;

/* default number of packets to send */
static GT_U32  prvTgfBurstCountArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    1, 1
};

/* VLAN Id */
static TGF_VLAN_ID  prvTgfVlanIdArr[] =
{
    1, 2
};

/* Counter Set ID array */
static GT_U8  prvTgfCntSetIdArr[] =
{
    0, 1
};

/* egress counters mode bitmap array */
static CPSS_PORT_EGRESS_CNT_MODE_ENT  prvTgfCntModeArr[] =
{
    CPSS_EGRESS_CNT_PORT_E, CPSS_EGRESS_CNT_VLAN_E
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
    {1, 1, 1, 0},
    {1, 1, 1, 0}
};

typedef GT_U32 egresssCountrArr_type[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_EGRESS_CNC_NUM_CNS];

/* expected number  */
static egresssCountrArr_type prvTgfBridgeEgressCountrArr =
{
    {0, 0, 0, 2, 0, 0, 0},
    {0, 0, 0, 2, 0, 1, 0}
};

/* expected number in case FROM_CPU packet is untagged */
static egresssCountrArr_type prvTgfBridgeEgressCountrArr_EtherTypeIgnore =
{
    {0, 0, 0, 2, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 0}
};


/* expected number for Puma3 - Fabric ports take in to account */
static egresssCountrArr_type prvTgfBridgeEgressCountr_Puma3Arr =
{
    {0, 0, 0, 2, 0, 0, 0},
    {0, 0, 0, 4, 0, 2, 0}
};

/* name array of egress counters */
static GT_CHAR   *tmpCharArr[PRV_TGF_EGRESS_CNC_NUM_CNS] =
{
    "brgEgrFilterDisc",
    "egrFrwDropFrames",
    "outBcFrames",
    "outCtrlFrames",
    "outMcFrames",
    "outUcFrames",
    "txqFilterDisc"
};

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                /* srcMac */
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
    GT_U16                                  vlanId;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   defInterface;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigSet
(
    GT_VOID
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  interface;
    GT_STATUS                              rc = GT_OK;
    GT_U8                                  tagArray[]  = {1, 1, 1, 1};
    GT_U8                                  confIter = 0;
    PRV_TGF_BRG_MAC_ENTRY_STC              macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* save default Vlan ID on port 0 */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfRestoreCfg.vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d, %d", prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* save default analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, prvTgfInterfaceIndex,
                                 &prvTgfRestoreCfg.defInterface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d,",
                                 prvTgfDevNum);

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

    /* describe Analyzer interface */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum = prvTgfDevNum;
    interface.interface.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    /* AUTODDOC: set analyzer interface on port 2 */
    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfInterfaceIndex, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d",
                                 prvTgfInterfaceIndex);

    for (confIter=0; confIter < PRV_TGF_CONF_ENTRY_NUM_CNS; confIter++)
    {
       /* clear entry */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* fill mac entry by defaults for this task */
        macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.isStatic                     = GT_TRUE;
        macEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
        macEntry.mirrorToRxAnalyzerPortEn     = GT_TRUE;
        macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
        macEntry.key.key.macVlan.vlanId       = prvTgfVlanIdArr[confIter];

        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 1, port 1 */
        /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 2, port 1 */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

        /* AUTODOC: set Egress Counters on Port 2 and Vlan 2 */
        rc = prvTgfPortEgressCntrModeSet(prvTgfCntSetIdArr[confIter], prvTgfCntModeArr[confIter],
                                         prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                         PRV_TGF_VLANID_CNS, 0, CPSS_DP_GREEN_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortEgressCntrModeSet");
    }
}

/**
* @internal prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U8                           portIter = 0;
    GT_U8                           sendIter = 0;
    GT_U8                           countIter = 0;
    GT_U32                          packetLen = 0;
    GT_U32                          tmpCountArr[PRV_TGF_EGRESS_CNC_NUM_CNS];
    CPSS_PORT_EGRESS_CNTR_STC       egrCntr = {0};
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs = {{{0}}};
    egresssCountrArr_type           *egresssCountrArrPtr;

    egresssCountrArrPtr = (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_PUMA3_E) ?
            &prvTgfBridgeEgressCountrArr:
            &prvTgfBridgeEgressCountr_Puma3Arr;

    if (tgfTrafficGeneratorEtherTypeIgnoreGet())
    {
        /* Not counting FROM CPU packet when it is untagged */
        egresssCountrArrPtr = &prvTgfBridgeEgressCountrArr_EtherTypeIgnore;
    }

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCommonAllCntrsReset");

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* set VlanId */
        cpssOsMemCpy(&prvTgfPacketVlanTagPart.vid, &prvTgfVlanIdArr[sendIter], sizeof(TGF_VLAN_ID));

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                                 prvTgfBurstCountArr[sendIter], 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: send packet on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:11:11, VID=1 */
        /* AUTODOC: send packet on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:11:11, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 and 2 packets - received on ports 1,2 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            packetLen = ((portIter == PRV_TGF_FDB_PORT_IDX_CNS) && (sendIter < 1)) ?
                prvTgfTaggedPacketInfo.totalLen - TGF_VLAN_TAG_SIZE_CNS :
                prvTgfTaggedPacketInfo.totalLen;

            /* read port counters  */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                           prvTgfPortsArray[portIter],
                                           GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "ERROR of prvTgfReadPortCountersEth Port=%02d\n",
                                         prvTgfPortsArray[portIter]);

            rc = (portCntrs.goodPktsRcv.l[0] ==
                  prvTgfPacketsCountRxArr[sendIter][portIter]*prvTgfBurstCountArr[sendIter]) ?
                  GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodPktsRcv counter than expected");

            rc = (portCntrs.goodPktsSent.l[0] ==
                  prvTgfPacketsCountTxArr[sendIter][portIter]*prvTgfBurstCountArr[sendIter]) ?
                  GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodPktsSent counter than expected");

            rc = (portCntrs.goodOctetsRcv.l[0] ==
                  (prvTgfPacketsCountRxArr[sendIter][portIter]*(packetLen + TGF_CRC_LEN_CNS)*prvTgfBurstCountArr[sendIter])) ?
                  GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodOctetsRcv counter than expected");

            rc = (portCntrs.goodOctetsSent.l[0] ==
                  (prvTgfPacketsCountTxArr[sendIter][portIter]*(packetLen + TGF_CRC_LEN_CNS)*prvTgfBurstCountArr[sendIter])) ?
                  GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodOctetsSent counter than expected");
        }
    }

    for (sendIter = 0; sendIter < PRV_TGF_CONF_ENTRY_NUM_CNS; sendIter++)
    {
        /* clear current counters value */
        cpssOsMemSet((GT_VOID*) &egrCntr, 0, sizeof(egrCntr));

        /* get egress counters */
        rc = prvTgfPortEgressCntrsGet(prvTgfCntSetIdArr[sendIter], &egrCntr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortEgressCntrsGet");

        tmpCountArr[0] = egrCntr.brgEgrFilterDisc;
        tmpCountArr[1] = egrCntr.egrFrwDropFrames;
        tmpCountArr[2] = egrCntr.outBcFrames;
        tmpCountArr[3] = egrCntr.outCtrlFrames;
        tmpCountArr[4] = egrCntr.outMcFrames;
        tmpCountArr[5] = egrCntr.outUcFrames;
        tmpCountArr[6] = egrCntr.txqFilterDisc;

        for (countIter = 0; countIter < PRV_TGF_EGRESS_CNC_NUM_CNS; countIter++)
        {
            /* AUTODOC: check egress counters  */
            rc = (tmpCountArr[countIter] ==
                 ((*egresssCountrArrPtr)[sendIter][countIter])) ?
                GT_OK : GT_FAIL;

            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                "counterSet[%d] : get another %s then expected: expected - %d, received - %d\n",
                sendIter,
                tmpCharArr[countIter],
                (*egresssCountrArrPtr)[sendIter][countIter],
                tmpCountArr[countIter]);
        }
    }
}

/**
* @internal prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgEgressCountersCtrlToAnlyzrPortVlanConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       confIter = 0;

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

    /* AUTODOC: restore analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfInterfaceIndex, &prvTgfRestoreCfg.defInterface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d",
                                 prvTgfInterfaceIndex);

    for (confIter = 0; confIter <PRV_TGF_CONF_ENTRY_NUM_CNS; confIter++)
    {
        /* AUTODOC: restore Egress Counters mode */
        rc = prvTgfPortEgressCntrModeSet(prvTgfCntSetIdArr[confIter], 0,
                                         prvTgfPortsArray[0],
                                         0, 0, CPSS_DP_GREEN_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfPortEgressCntrModeSet");
    }
}


