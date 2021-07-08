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
* @file prvTgfTailDropTxqUtil.c
*
* @brief "TxQ utilization" enhanced
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfCosGen.h>
#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>

#define PRV_TGF_RECV_PORT_IDX_CNS 1
#define PRV_TGF_SEND_PORT_IDX_CNS 0
#define PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS 13
#define PRV_TGF_VLANID_CNS 7
#define PRV_TGF_RCVPRT_COS_PROFILE_CNS 628
#define PRV_TGF_TC_COUNT_CNS 8

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x01, 0x02, 0x03, 0xAA, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
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
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/**
* @internal prvTgfPortTxTailDropTxqUtilConfigure function
* @endinternal
*
* @brief Prepare or restore prvTgfPortTxTailDropTxqUtil test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID prvTgfTailDropTxqUtilConfigure
(
    GT_BOOL configSet
)
{
    GT_STATUS   rc;
    static CPSS_QOS_ENTRY_STC                   qosRecvPortConfigRestore;
    CPSS_QOS_ENTRY_STC                          qosRecvPortConfig;
    static PRV_TGF_COS_PROFILE_STC              cosProfileRestore;
    static CPSS_QOS_PORT_TRUST_MODE_ENT         trustModeRestore;
    GT_U8                                       vlanTags[2];
    static GT_BOOL                              enPortStatRestore;
    static GT_BOOL                              enQueueStatRestore;

    if(configSet)
    {
        /* AUTODOC: set vlan entry */
        vlanTags[0] = 1;
        vlanTags[1] = 1;
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
            NULL, vlanTags, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_CNS);

        /* AUTODOC: Save default port QoS profile */
        rc = prvTgfCosPortQosConfigGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &qosRecvPortConfigRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigGet");

        /* AUTODOC: Set default QoS profile for recv port */
        qosRecvPortConfig.qosProfileId = PRV_TGF_RCVPRT_COS_PROFILE_CNS;
        qosRecvPortConfig.assignPrecedence =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        qosRecvPortConfig.enableModifyUp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        qosRecvPortConfig.enableModifyDscp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        rc = prvTgfCosPortQosConfigSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &qosRecvPortConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

        /* AUTODOC: Save trust mode */
        rc = prvTgfCosPortQosTrustModeGet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &trustModeRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosTrustModeGet %d",
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]);

        /* AUTODOC: Set trust mode NO TRUST */
        rc = prvTgfCosPortQosTrustModeSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_QOS_PORT_NO_TRUST_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosTrustModeSet %d %d",
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], CPSS_QOS_PORT_NO_TRUST_E);

        /* AUTODOC: Save CoS entry */
        rc = prvTgfCosProfileEntryGet(prvTgfDevNum,
            PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Save statistics enabled state */
        rc = prvTgfPortTxBufferStatisticsEnableGet(prvTgfDevNum,
            &enPortStatRestore, &enQueueStatRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBufferStatisticsEnableGet %d",
            prvTgfDevNum);

        /* AUTODOC: Configure statistics enabled state */
        rc = prvTgfPortTxBufferStatisticsEnableSet(prvTgfDevNum,
            GT_TRUE, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBufferStatisticsEnableSet %d %d %d",
            prvTgfDevNum, GT_TRUE, GT_TRUE);

    }
    else
    {
        /* AUTODOC: Save statistics enabled state */
        rc = prvTgfPortTxBufferStatisticsEnableSet(prvTgfDevNum,
            enPortStatRestore, enQueueStatRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTxBufferStatisticsEnableSet %d",
            prvTgfDevNum, enPortStatRestore, enQueueStatRestore);

        /* AUTODOC: Restore CoS entry */
        rc = prvTgfCosProfileEntrySet(PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Restore trust mode */
        rc = prvTgfCosPortQosTrustModeSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            trustModeRestore);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosTrustModeSet %d %d",
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], trustModeRestore);

        /* AUTODOC: Restore default port QoS profile */
        rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &qosRecvPortConfigRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

        /* AUTODOC: Delete static and dynamic FDB entry */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
            prvTgfDevNum, GT_TRUE);

        rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d",
            PRV_TGF_VLANID_CNS);

        /* AUTODOC: Reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");
    }
}

/**
* @internal prvTgfPortTxTailDropTxqUtilTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot prvTgfPortTxTailDropTxqUtil test.
*
*/
GT_VOID prvTgfTailDropTxqUtilTrafficGenerate()
{
    GT_U32 expectedValue = 0;
    GT_STATUS st;
    CPSS_PORT_MAC_COUNTER_SET_STC portCounters;
    GT_U32 portMax;
    GT_U32 queueMax;
    GT_PHYSICAL_PORT_NUM port;
    GT_U8  tc;
    PRV_TGF_COS_PROFILE_STC cosProfile;
    GT_U32  numOfPackets;
    GT_U8 current_tc;
    GT_U32 numOfBuffs;

    for(current_tc = 0; current_tc < PRV_TGF_TC_COUNT_CNS; current_tc++)
    {
        /* AUTODOC: Clear counters */
        st = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

        /* AUTODOC: Clear statistics */
        st = prvTgfPortTxMaxBufferFillLevelSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], 0);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "prvTgfPortTxMaxBufferFillLevelSet %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], 0);

        st = prvTgfPortTxQueueMaxBufferFillLevelSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], current_tc, 0);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueMaxBufferFillLevelSet %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], 0);

        /*  AUTODOC: Configure COS profile 628 with Drop Precedence and
            Traffic Class of ingress frames. */
        cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));
        cosProfile.trafficClass = current_tc;
        st = prvTgfCosProfileEntrySet(PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfile);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfCosProfileEntrySet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Block TxQ for receive port (1).*/
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            current_tc, GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_RCVPRT_COS_PROFILE_CNS, GT_FALSE);

        /* AUTODOC: Transmit frames (with MAC DA 00:01:02:03:AA:01) to
            port 0. */
        numOfPackets = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS + current_tc;
        st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
            numOfPackets, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
            numOfPackets, 0, NULL);

        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, 1);

        st = cpssDxChPortTxBufNumberGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], &numOfBuffs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChPortTxBufNumberGet %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]);

        PRV_UTF_LOG1_MAC("Number of TX Buffers: %d\n",numOfBuffs);

        /* AUTDOC: Release TxQ for receive port (1) */
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            current_tc, GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_RCVPRT_COS_PROFILE_CNS, GT_TRUE);

        cpssOsTimerWkAfter(1);

        /* AUTODOC: Check MAC counters. */
        st = prvTgfReadPortCountersEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], GT_FALSE,
            &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfReadPortCountersEth");

        expectedValue = numOfPackets;
        if (portCounters.goodPktsSent.l[0] != expectedValue)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(expectedValue, portCounters.goodPktsSent.l[0],
                    "Got other counter value than expected:\n"
                    "   Tx counter:      %d\n",
                    portCounters.goodPktsSent.l[0]);
        }
        else
        {
            PRV_UTF_LOG1_MAC("Got Tx counter value as expected:\n"
                                "   Tx counter:      %d\n",
                                portCounters.goodPktsSent.l[0]);
        }

        expectedValue--;

        /* AUTODOC: Check statistics */
        for(port = 0; port < prvTgfPortsNum; port++)
        {
            st = prvTgfPortTxMaxBufferFillLevelGet(prvTgfDevNum,
                prvTgfPortsArray[port], &portMax);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "prvTgfPortTxMaxBufferFillLevelSet %d %d",
                prvTgfDevNum, prvTgfPortsArray[port]);

            if(PRV_TGF_RECV_PORT_IDX_CNS == port)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, portMax, "Unexpected peak value");
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portMax, "Unexpected peak value");
            }

            st = prvTgfPortTxMaxBufferFillLevelSet(prvTgfDevNum,
                prvTgfPortsArray[port], 0);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "prvTgfPortTxMaxBufferFillLevelSet %d %d %d",
                prvTgfDevNum, prvTgfPortsArray[port], 0);

            for(tc = 0; tc < PRV_TGF_TC_COUNT_CNS; tc++)
            {
                st = prvTgfPortTxQueueMaxBufferFillLevelGet(prvTgfDevNum,
                    prvTgfPortsArray[port], tc, &queueMax);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "prvTgfPortTxQueueMaxBufferFillLevelSet %d %d",
                    prvTgfDevNum, prvTgfPortsArray[port]);

                if((current_tc == tc) && (PRV_TGF_RECV_PORT_IDX_CNS == port))
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, queueMax, "Unexpected peak value");
                }
                else
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, queueMax, "Unexpected peak value");
                }

                st = prvTgfPortTxQueueMaxBufferFillLevelSet(prvTgfDevNum,
                    prvTgfPortsArray[port], tc, 0);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "prvTgfPortTxQueueMaxBufferFillLevelSet %d %d %d",
                    prvTgfDevNum, prvTgfPortsArray[port], 0);
            }
        }
    }
}
