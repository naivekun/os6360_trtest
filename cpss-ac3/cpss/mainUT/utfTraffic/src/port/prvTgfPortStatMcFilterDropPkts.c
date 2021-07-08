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
* @file prvTgfPortStatMcFilterDropPkts.c
*
* @brief Check filtered multicast drop counters
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <port/prvTgfPortStatMcFilterDropPkts.h>

#define PRV_TGF_VLANID_5_CNS            5
#define PRV_TGF_SEND_PORT_IDX_CNS       0
#define PRV_TGF_RECV_PORT_1_CNS         1
#define PRV_TGF_RECV_PORT_2_CNS         2
#define PRV_TGF_RECV_PORT_3_CNS         3
#define PRV_TGF_RECV_PORT_NUM           3

static GT_U32 prvTgfBurstCount = 1;
static GT_U32 prvTgfRcvPorts[PRV_TGF_RECV_PORT_NUM] = {
    PRV_TGF_RECV_PORT_1_CNS,
    PRV_TGF_RECV_PORT_2_CNS,
    PRV_TGF_RECV_PORT_3_CNS
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x5E, 0x00, 0x00, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x04, 0x07, 0x0a, 0x0d, 0x01, 0x04, 0x07,
    0x02, 0x05, 0x08, 0x0b, 0x0e, 0x02, 0x05, 0x08,
    0x03, 0x06, 0x09, 0x0c, 0x0f, 0x03, 0x06, 0x09,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x43
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

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/**
* @internal prvTgfPortStatMcFilterDropPktsConfigure function
* @endinternal
*
* @brief   Configure or restore test configuration
*
* @param[in] config - GT_TRUE  - configure
*                     GT_FALSE - restore
*
*/
GT_VOID prvTgfPortStatMcFilterDropPktsConfigure
(
    GT_BOOL config
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {1, 1, 1, 1};
    GT_BOOL     restoreEnableUnregMcPort[PRV_TGF_RECV_PORT_NUM] =
        {GT_FALSE, GT_FALSE, GT_FALSE};
    GT_U32 ii;

    if(GT_TRUE == config)
    {
        /* AUTODOC: create VLAN 5 */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS,
            prvTgfPortsArray, NULL, tagArray, 4);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

        /* AUTODOC: save MC drop configuration */
        for(ii = 0; ii < PRV_TGF_RECV_PORT_NUM; ii++)
        {
            rc = prvTgfBrgEgressPortUnregMcFilteringGet(prvTgfDevNum,
                prvTgfPortsArray[prvTgfRcvPorts[ii]],
                &restoreEnableUnregMcPort[ii]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfBrgEgressPortUnregMcFilteringGet: %d %d", prvTgfDevNum,
                prvTgfPortsArray[prvTgfRcvPorts[ii]]);
        }

        /* AUTODOC: set MC drop configuration */
        for(ii = 0; ii < PRV_TGF_RECV_PORT_NUM; ii++)
        {
            rc = prvTgfBrgEgressPortUnregMcFilteringSet(
                prvTgfPortsArray[prvTgfRcvPorts[ii]], GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfBrgEgressPortUnregMcFilteringSet: %d %d",
                prvTgfPortsArray[prvTgfRcvPorts[ii]], GT_TRUE);
        }
    }
    else
    {
        /* AUTODOC: Clear FDB */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* AUTODOC: flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

        /* AUTODOC: restore MC drop configuration */
        for(ii = 0; ii < PRV_TGF_RECV_PORT_NUM; ii++)
        {
            rc = prvTgfBrgEgressPortUnregMcFilteringSet(
                prvTgfPortsArray[prvTgfRcvPorts[ii]],
                restoreEnableUnregMcPort[ii]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfBrgEgressPortUnregMcFilteringSet: %d %d",
                prvTgfPortsArray[prvTgfRcvPorts[ii]],
                restoreEnableUnregMcPort[ii]);
        }

        /* AUTODOC: invalidate VLAN entry 5 */
        rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);
    }
}

struct {
    GT_U32 mcFilterDropPkts;
    GT_U32 outUcFrames;
    GT_U32 outMcFrames;
    GT_U32 mcPktsSent[PRV_TGF_RECV_PORT_NUM];
} prvTgfCounters[2] =
{
    {1, 0, 0, {0, 0, 0}},
    {0, 0, 1, {1, 0, 0}}
};

/**
* @internal prvTgfPortStatMcFilterDropPktsGenerate function
* @endinternal
*
* @brief   Send test traffic and validate counters
*
*
*/
GT_VOID prvTgfPortStatMcFilterDropPktsGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_PORT_EGRESS_CNTR_STC counters;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32 ii;
    GT_U32 jj;

    /* AUTODOC: setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, &prvTgfPacketInfo,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: Clear counters through read */
    rc = prvTgfPortEgressCntrsGet(0/* Set 0 */, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

    /* AUTODOC: Clear MAC counters */
    for(jj = 0; jj < PRV_TGF_RECV_PORT_NUM; jj++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
            prvTgfPortsArray[prvTgfRcvPorts[jj]], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[prvTgfRcvPorts[jj]]);
    }

    for(ii = 0; ii < 2; ii++)
    {
        /* AUTODOC: start transmitting */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

        /* AUTODOC: Read counters for port*/
        rc = prvTgfPortEgressCntrsGet(0/* Set 0 */, &counters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet");

        /* AUTODOC: Validate MC Filter drop counter */
        UTF_VERIFY_EQUAL0_STRING_MAC(
            prvTgfBurstCount * prvTgfCounters[ii].mcFilterDropPkts,
            counters.mcFilterDropPkts, "Unexpected MC Filter packets counter");

        /* AUTODOC: Validate Out UC packetes */
        UTF_VERIFY_EQUAL0_STRING_MAC(
            prvTgfBurstCount * prvTgfCounters[ii].outUcFrames,
            counters.outUcFrames, "Unexpected UC packets counter");

        /* AUTODOC: Validate Out MC packetes */
        UTF_VERIFY_EQUAL0_STRING_MAC(
            prvTgfBurstCount * prvTgfCounters[ii].outMcFrames,
            counters.outMcFrames, "Unexpected MC packets counter");

        /* AUTODOC: Check MAC counters */
        for(jj = 0; jj < PRV_TGF_RECV_PORT_NUM; jj++)
        {
            rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                prvTgfPortsArray[prvTgfRcvPorts[jj]], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfReadPortCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[prvTgfRcvPorts[jj]]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                prvTgfBurstCount * prvTgfCounters[ii].mcPktsSent[jj],
                portCntrs.mcPktsSent.l[0],
                "Unexpected MC packets MAC counter for port %d",
                prvTgfRcvPorts[jj]);
        }

        /* AUTODOC: Allow MC traffic on receive port 1 */
        rc = prvTgfBrgEgressPortUnregMcFilteringSet(
            prvTgfPortsArray[PRV_TGF_RECV_PORT_1_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfBrgEgressPortUnregMcFilteringSet: %d %d",
            prvTgfPortsArray[PRV_TGF_RECV_PORT_1_CNS], GT_FALSE);
    }
}
