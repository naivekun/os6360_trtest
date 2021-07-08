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
* @file prvTgfOamGeneral.c
*
* @brief Common OAM features testing
*
* @version   23
********************************************************************************
*/
#include <oam/prvTgfOamGeneral.h>
#include <common/tgfVntGen.h>

/****************** packet 1 ********************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_8902_CFM_TAG_CNS};

/* DATA of OAM header */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};


/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    sizeof(prvTgfPayloadDataArr))

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* OAM IPCL Lookup Configuration */
PRV_TGF_OAM_PCL_RULE_CONFIG_STC oamIPclRuleInfo[] =
{
    {CPSS_INTERFACE_PORT_E,  PRV_TGF_SEND_PORT_IDX_CNS,    PRV_TGF_MEL_BYTE_3_CNS},
    {CPSS_INTERFACE_PORT_E,  PRV_TGF_SEND_PORT_IDX_CNS,    PRV_TGF_MEL_BYTE_5_CNS},
    {CPSS_INTERFACE_PORT_E,  PRV_TGF_SEND_PORT_IDX_CNS,    PRV_TGF_MEL_BYTE_DONT_CARE_CNS},
    {CPSS_INTERFACE_TRUNK_E, PRV_TGF_FDB_PORT_0_IDX_CNS,   PRV_TGF_MEL_BYTE_5_CNS},
    {CPSS_INTERFACE_PORT_E,  PRV_TGF_RECEIVE_PORT_IDX_CNS, PRV_TGF_MEL_BYTE_3_CNS},
    {CPSS_INTERFACE_PORT_E,  PRV_TGF_RECEIVE_PORT_IDX_CNS, PRV_TGF_MEL_BYTE_DONT_CARE_CNS},
};

/* OAM EPCL Lookup Configuration */
PRV_TGF_OAM_EPCL_RULE_CONFIG_STC oamEPclRuleInfo[] =
{
    /* Delay Measurement EPCL entries */
    {CPSS_INTERFACE_PORT_E, PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
        PRV_TGF_DM_OPCODE_BYTE_45_CNS, PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E},
    {CPSS_INTERFACE_PORT_E, PRV_TGF_CPU_PORT_IDX_CNS,
        PRV_TGF_DM_OPCODE_BYTE_45_CNS, PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E},
    {CPSS_INTERFACE_PORT_E, PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
        PRV_TGF_DM_OPCODE_BYTE_46_CNS, PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E},
    {CPSS_INTERFACE_PORT_E, PRV_TGF_CPU_PORT_IDX_CNS,
        PRV_TGF_DM_OPCODE_BYTE_46_CNS, PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E},
    /* Loss Measurement EPCL entries */
    {CPSS_INTERFACE_PORT_E, PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
        PRV_TGF_LM_OPCODE_BYTE_42_CNS, PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E},
    {CPSS_INTERFACE_PORT_E, PRV_TGF_CPU_PORT_IDX_CNS,
        PRV_TGF_LM_OPCODE_BYTE_42_CNS, PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E},
    {CPSS_INTERFACE_PORT_E, PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
        PRV_TGF_LM_OPCODE_BYTE_43_CNS, PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E},
    {CPSS_INTERFACE_PORT_E, PRV_TGF_CPU_PORT_IDX_CNS,
        PRV_TGF_LM_OPCODE_BYTE_43_CNS, PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E},
};

/* EPCL rules numbers */
GT_U32 oamEPclRulesNum =
    sizeof(oamEPclRuleInfo) / sizeof(PRV_TGF_OAM_EPCL_RULE_CONFIG_STC);

/* Captured packets */
TGF_PACKET_STC * capturePacketsArray[PRV_TGF_OAM_BURST_COUNT_CNS];

/* EPCL ports array */
static GT_U32   epclPortsArray[PRV_TGF_EPCL_PORTS_NUM_CNS];

/* CPU port number */
#define PRV_CPSS_CPU_PORT_NUM_CNS   63

/* Burst packets number */
GT_U32 prvTgfOamBurstCount = PRV_TGF_OAM_BURST_COUNT_CNS;

/* flag to state that we use also TTI for the OAM profile assignment */
static GT_BOOL  testUseTtiForOam = GT_FALSE;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfOamPacketHeaderDataSet function
* @endinternal
*
* @brief   Set OAM packet header part
*
* @param[in] packetHeaderPartType     - packet header part type
* @param[in,out] packetHeaderPartPtr      - pointer to packet header part
*                                       None
*/
GT_VOID prvTgfOamPacketHeaderDataSet
(
    IN    TGF_PACKET_PART_ENT  packetHeaderPartType,
    INOUT GT_VOID             *packetHeaderPartPtr
)
{
    switch(packetHeaderPartType)
    {
        case TGF_PACKET_PART_L2_E:
            cpssOsMemCpy(prvTgfPacketPartArray[0].partPtr,
                         packetHeaderPartPtr,
                         sizeof(TGF_PACKET_L2_STC));
            break;
        case TGF_PACKET_PART_ETHERTYPE_E:
            cpssOsMemCpy(prvTgfPacketPartArray[2].partPtr,
                         packetHeaderPartPtr,
                         sizeof(TGF_PACKET_ETHERTYPE_STC));
            break;
        default:
            break;
    }
}

/**
* @internal prvTgfOamPacketHeaderDataReset function
* @endinternal
*
* @brief   Reset OAM packet header part
*/
GT_VOID prvTgfOamPacketHeaderDataReset
(
)
{
    TGF_PACKET_L2_STC prvTgfPacketL2Part = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x18},               /* dstMac */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                /* srcMac */
    };

    /* First VLAN_TAG part */
    TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
        0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
    };

    /* packet's ethertype */
    TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
        TGF_ETHERTYPE_8902_CFM_TAG_CNS};

    /* Restore MAC addreses */
    cpssOsMemCpy(prvTgfPacketPartArray[0].partPtr,
                 &prvTgfPacketL2Part,
                 sizeof(TGF_PACKET_L2_STC));
    /* Restore Vlan */
    cpssOsMemCpy(prvTgfPacketPartArray[1].partPtr,
                 &prvTgfPacketVlanTag0Part,
                 sizeof(TGF_PACKET_VLAN_TAG_STC));
    /* Restore ethertype */
    cpssOsMemCpy(prvTgfPacketPartArray[2].partPtr,
                 &prvTgfPacketEtherTypePart,
                 sizeof(TGF_PACKET_ETHERTYPE_STC));

    /* Reset payload */
    cpssOsMemSet(prvTgfPacketPayloadPart.dataPtr, 0,
                    prvTgfPacketPayloadPart.dataLength);
}

/**
* @internal prvTgfOamPduHeaderDataSet function
* @endinternal
*
* @brief   Set OAM PDU header buffer
*
* @param[in] pduHeaderDataPtr         - pointer to PDU header data structure
* @param[in,out] pduHeaderBuffPtr         - pointer to PDU header buffer
* @param[in,out] pduHeaderBuffPtr         - pointer to PDU header buffer
*                                       None
*/
static GT_VOID prvTgfOamPduHeaderDataSet
(
    IN    PRV_TGF_OAM_PDU_HEADER_DATA_STC  * pduHeaderDataPtr,
    INOUT GT_U8                            * pduHeaderBuffPtr
)
{
    /* octet 0 - MEL + Version */
    pduHeaderBuffPtr[0] =
        (pduHeaderDataPtr->megLevel << 5) | (pduHeaderDataPtr->version & 0x1f);
    /* octet 1 - Opcode */
    pduHeaderBuffPtr[1] = pduHeaderDataPtr->opcode;
    /* octet 2 - Flags */
    pduHeaderBuffPtr[2] = pduHeaderDataPtr->flags;
    /* octet 3 - TLV Offset */
    pduHeaderBuffPtr[3] = pduHeaderDataPtr->tlvOffset;
    if(pduHeaderDataPtr->tlvOffset)
    {
        /* Copy the rest of data to buffer starting from offset 4 */
        cpssOsMemCpy(&pduHeaderBuffPtr[4], pduHeaderDataPtr->tlvDataArr,
                    pduHeaderDataPtr->tlvOffset);
    }
}
/**
* @internal prvTgfOamTransmitSetup function
* @endinternal
*
* @brief   Set transmission parameters
*
* @param[in] pduHeaderDataPtr         - pointer to PDU header data structure
* @param[in] sendPort                 - send port
*                                       None
*/
GT_VOID prvTgfOamTransmitSetup
(
    IN  PRV_TGF_OAM_PDU_HEADER_DATA_STC  * pduHeaderDataPtr,
    IN  GT_U8 sendPort
)
{
    GT_STATUS  rc = GT_OK;
    GT_U8      portIter;                /* port iterator */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == PRV_CPSS_CPU_PORT_NUM_CNS)
        {
            continue;
        }
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                    &prvTgfPacketInfo,
                    prvTgfOamBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                    prvTgfDevsArray[sendPort],
                    prvTgfOamBurstCount, 0, NULL);

    /* set capturing mode */
    rc = tgfTrafficTableRxModeSet(TGF_RX_CYCLIC_E, 600);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "tgfTrafficTableRxModeSet: %d",
                    prvTgfDevsArray[sendPort]);

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "tgfTrafficTableRxStartCapture");

    /* Set OAM PDU header */
    prvTgfOamPduHeaderDataSet(pduHeaderDataPtr, prvTgfPayloadDataArr);
}

/**
* @internal prvTgfOamTransmitPacket function
* @endinternal
*
* @brief   Set transmission parameters and send packets
*
* @param[in] pduHeaderDataPtr         - pointer to PDU header data structure
* @param[in] sendPort                 - send port
* @param[in] burstCount               - packet burst count
* @param[in] sleepTime                - transmit timings in milliseconds
*                                       None
*/
GT_VOID prvTgfOamTransmitPacket
(
    IN  PRV_TGF_OAM_PDU_HEADER_DATA_STC  * pduHeaderDataPtr,
    IN  GT_U8 sendPort,
    IN  GT_U32 burstCount,
    IN  GT_U32 sleepTime
)
{
    GT_STATUS rc;

    /* Set packets burst count */
    prvTgfOamBurstCount = burstCount;

    /* Set OAM transmitt parameters */
    prvTgfOamTransmitSetup(pduHeaderDataPtr, sendPort);

    /* Set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, sleepTime);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);
    /* Send Packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPort]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                    prvTgfDevNum,
                    prvTgfPortsArray[sendPort]);
}

/**
* @internal prvTgfOamRxInCpuCheck function
* @endinternal
*
* @brief   Check Rx CPU packets
*
* @param[in] expectedPackets          - expected packets captured by CPU
*                                       None
*/
static GT_VOID prvTgfOamRxInCpuCheck
(
    IN GT_U32 expectedPackets
)
{
    GT_STATUS       rc = GT_OK;
    TGF_NET_DSA_STC rxParam;            /* Rx network inetrface DSA tag parameters */
    GT_U8           devNum   = 0;       /* device number */
    GT_U8           queue    = 0;       /* Rx traffic queue */
    GT_BOOL         getFirst = GT_TRUE; /* Get Rx packet strating from first */
    GT_U32          buffLen  = 0x600;   /* Rx buffer length */
    GT_U32          packetActualLength = 0; /* Rx packet's length  */
    static  GT_U8   packetBuff[0x600] = {0};/* Rx packet buffer */
    GT_U32          packetCount;        /* packets counter */

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "tgfTrafficTableRxStartCapture");

    cpssOsMemSet(capturePacketsArray, 0, sizeof(capturePacketsArray));

    for(packetCount = 0; packetCount < expectedPackets; packetCount++)
    {
        buffLen  = 0x600;
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                    getFirst, GT_TRUE, packetBuff,
                    &buffLen, &packetActualLength,
                    &devNum, &queue, &rxParam);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc ,
                    "tgfTrafficGeneratorRxInCpuGet %d",
                    TGF_PACKET_TYPE_CAPTURE_E);

        rc = tgfTrafficEnginePacketParse(packetBuff, buffLen,
                    &capturePacketsArray[packetCount]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc ,
                    "tgfTrafficEnginePacketParse %d",
                    TGF_PACKET_TYPE_CAPTURE_E);

        getFirst = GT_FALSE; /* now we get the next */
    }

    /* check expected Rx packets */
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedPackets, packetCount,
                    "Rx packets differ than excpected");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(
                    GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfOamPortCountersForwardCheck function
* @endinternal
*
* @brief   Check ports counters results on forward packets
*
* @param[in] sendPort                 - port index to send traffic to
* @param[in] expectedPackets          - expected forwarded packets
*                                       None
*/
static GT_VOID prvTgfOamPortCountersForwardCheck
(
    IN GT_U32 sendPort,
    IN GT_U32 expectedPackets
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portIter   = 0;         /* port iterator */
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;/* port's MAC counters */
    GT_U32          expectedTxPacketTrunk = 0; /* excpected Tx packet on trunk */

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == PRV_CPSS_CPU_PORT_NUM_CNS)
        {
            continue;
        }
        /* read counters */
        rc = prvTgfReadPortCountersEth(
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter],
                    GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);

        if (sendPort != portIter &&
            sendPort != PRV_TGF_FDB_PORT_0_IDX_CNS && sendPort != PRV_TGF_FDB_PORT_3_IDX_CNS)
        {
            switch(portIter)
            {
                case PRV_TGF_FDB_PORT_0_IDX_CNS:    /* Trunk Port */
                    expectedTxPacketTrunk += portCntrs.goodPktsSent.l[0];
                    continue;
                case PRV_TGF_FDB_PORT_3_IDX_CNS:    /* Trunk Port */
                    expectedTxPacketTrunk += portCntrs.goodPktsSent.l[0];
                    /* Check number of transmitted packets on trunk */
                    UTF_VERIFY_EQUAL2_STRING_MAC(expectedPackets,
                    expectedTxPacketTrunk,
                    "get another goodPktsSent counter than expected: %d, %d\n",
                    expectedPackets, expectedTxPacketTrunk);
                    break;
                default:
                    /* Check number of transmitted packets */
                    UTF_VERIFY_EQUAL2_STRING_MAC(expectedPackets, portCntrs.goodPktsSent.l[0],
                    "get another goodPktsSent counter than expected: %d, %d\n",
                    expectedPackets, portCntrs.goodPktsSent.l[0]);
            }
        }
    }
}

/**
* @internal prvTgfOamExceptionTableCheck function
* @endinternal
*
* @brief   Check exception counters and tables
*
* @param[in] oamExceptionType         - exception type
* @param[in] expectedCounters         - number of expected exception counters
*                                       None
*/
static GT_VOID prvTgfOamExceptionTableCheck
(
    IN PRV_TGF_OAM_EXCEPTION_TYPE_ENT  oamExceptionType,
    IN GT_U32 expectedCounters
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    counterValue;             /* counter value */
    GT_U32    entryBmp[1];              /* exception entry bitmap */
    GT_U32    exceptionBit;             /* exception summary bit */
    GT_U32    readCounters;             /* read counters tries */

    if(oamExceptionType == PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E)
    {
        /* Exception not expected */
        return;
    }

    rc = prvTgfOamExceptionCounterGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamExceptionType, &counterValue);

    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                    "prvTgfOamExceptionCounterGet: %d, %d",
                    prvTgfDevNum,
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamExceptionType);

    if(oamExceptionType == PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_AGING_E)
    {
        readCounters = 10;
        /* Wait until counter get value */
        while(counterValue == 0)
        {
            rc = prvTgfOamExceptionCounterGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                            oamExceptionType, &counterValue);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                            "prvTgfOamExceptionCounterGet: %d, %d",
                            prvTgfDevNum,
                            PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                            oamExceptionType);
            if(readCounters--)
            {
                /* Wait 200 miliseconds */
                cpssOsTimerWkAfter(200);
            }
            else
            {
                /* HW error (should not happen) */
                break;
            }
        }
        /* check exception counters - could be more than excpected in keepalive aging */
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, counterValue,
                        "Check exception counters: exception %d",
                        oamExceptionType);
    }
    else
    {
        /* check exception counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(expectedCounters, counterValue,
                        "Check exception counters: exception %d",
                        oamExceptionType);
    }

    if(expectedCounters)
    {
        prvTgfOamExceptionStatusGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamExceptionType, 0, &entryBmp[0]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                    "prvTgfOamExceptionStatusGet: %d, %d",
                    prvTgfDevNum,
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamExceptionType);

        exceptionBit = (entryBmp[0] == 0) ? 0 : 1;
        /* check exception bit */
        UTF_VERIFY_EQUAL1_STRING_MAC(1, exceptionBit, "Exception %d bit not set",
                    oamExceptionType);
    }
}

/**
* @internal prvTgfOamIngressBillingCounterCheck function
* @endinternal
*
* @brief   Check ingress billing counters results
*
* @param[in] excpectedBillingCounters - excpected billing counter
*                                       None
*/
static GT_VOID prvTgfOamIngressBillingCounterCheck
(
    IN GT_U32   excpectedBillingCounters
)
{
    GT_STATUS rc = GT_OK;

    PRV_TGF_POLICER_BILLING_ENTRY_STC     billingCntr;  /* billing entry */
    PRV_TGF_POLICER_BILLING_ENTRY_STC     readBillingCntr; /* billing entry */

    billingCntr.greenCntr.l[0] = excpectedBillingCounters;

    /* Read Billing Entry */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    GT_TRUE,
                    &readBillingCntr);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                    "cpssDxCh3PolicerBillingEntryGet: %d, "
                    "policerStage = %d, entryIndex = %d",
                    prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Check counters */
    UTF_VERIFY_EQUAL4_STRING_MAC(billingCntr.greenCntr.l[0],
                    readBillingCntr.greenCntr.l[0],
                    "billingCntr.greenCntr.l[0] = %d, "
                    "readBillingCntr.greenCntr.l[0] = %d, "
                    "policerStage = %d, entryIndex = %d\n",
                    billingCntr.greenCntr.l[0],
                    readBillingCntr.greenCntr.l[0],
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfOamRxPduHeaderParse function
* @endinternal
*
* @brief   Parse OAM PDU header from Rx table
*
* @param[in] rxPacketPtr              - pointer to Rx packet
* @param[in,out] pduHeaderDataPtr         - pointer to PDU header data structure
* @param[in,out] pduHeaderDataPtr         - pointer to PDU header data structure
*                                       None
*/
GT_VOID prvTgfOamRxPduHeaderParse
(
    IN    TGF_PACKET_STC * rxPacketPtr,
    INOUT PRV_TGF_OAM_PDU_HEADER_DATA_STC * pduHeaderDataPtr
)
{
    GT_U32                  pktPart;        /* packet part number */
    TGF_PACKET_PAYLOAD_STC  * payloadPartPtr = 0; /* pointer to payload part */
    TGF_PACKET_ETHERTYPE_STC * etherTypePartPtr = 0; /* pointer to ethertype */
    TGF_PACKET_PART_STC     *pktPartPtr;    /* pointer to packet part */

    /* Get pointer to packet's array */
    pktPartPtr = rxPacketPtr->partsArray;

    /* Search for payload part in CPU Rx packet */
    for(pktPart = 0; pktPart < rxPacketPtr->numOfParts; pktPart++)
    {
        switch(pktPartPtr[pktPart].type)
        {
            case TGF_PACKET_PART_ETHERTYPE_E:
                etherTypePartPtr =
                    (TGF_PACKET_ETHERTYPE_STC *)pktPartPtr[pktPart].partPtr;
                break;
            case TGF_PACKET_PART_PAYLOAD_E:
                payloadPartPtr =
                    (TGF_PACKET_PAYLOAD_STC *)pktPartPtr[pktPart].partPtr;
                break;
            default:
                continue;
        }
    }
    if(etherTypePartPtr == 0)
    {
        PRV_UTF_LOG0_MAC("prvTgfOamRxPduHeaderParse:  Ethertype not found");
        return;
    }

    if(etherTypePartPtr->etherType != TGF_ETHERTYPE_8902_CFM_TAG_CNS)
    {
        PRV_UTF_LOG1_MAC("Not expected ethertype:  %d\n", etherTypePartPtr->etherType);
        return;
    }

    if(payloadPartPtr == 0)
    {
        PRV_UTF_LOG0_MAC("prvTgfOamRxPduHeaderParse: Payload not found\n");
        return;
    }


    pduHeaderDataPtr->megLevel =
                    payloadPartPtr->dataPtr[PRV_TGF_OAM_MEL_OFFSET_STC] >> 5;
    pduHeaderDataPtr->version =
                    payloadPartPtr->dataPtr[PRV_TGF_OAM_MEL_OFFSET_STC] & 0x1f;
    pduHeaderDataPtr->opcode =
                    payloadPartPtr->dataPtr[PRV_TGF_OAM_OPCODE_OFFSET_STC];
    pduHeaderDataPtr->flags =
                    payloadPartPtr->dataPtr[PRV_TGF_OAM_FLAGS_OFFSET_STC];
    pduHeaderDataPtr->tlvOffset =
                    payloadPartPtr->dataPtr[PRV_TGF_OAM_TLV_OFFSET_STC];
    if(pduHeaderDataPtr->tlvOffset)
    {
        /* Copy the timestamp data to buffer */
        cpssOsMemCpy(&pduHeaderDataPtr->tlvDataArr,
                    &payloadPartPtr->dataPtr[PRV_TGF_OAM_TLV_DATA_OFFSET_STC],
                    pduHeaderDataPtr->tlvOffset);
    }
}

/**
* @internal prvTgfOamBridgeConfigurationSet function
* @endinternal
*
* @brief   Set OAM test bridge configuration
*/
GT_VOID prvTgfOamBridgeConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[2]; /* trunk members array */
    GT_TRUNK_ID             trunkId;        /* trunk Id */

    /* Create Vlan 5 on ports with indexs 0, 1, 2, 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
                    prvTgfPortsArray, NULL, NULL,
                    prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfBrgDefVlanEntryWithPortsSet: Vlan %d",
                    PRV_TGF_VLANID_CNS);

    /* Create trunk 6 with port members 0, 2 */
    enabledMembersArray[0].hwDevice = prvTgfDevsArray[PRV_TGF_FDB_PORT_0_IDX_CNS];
    enabledMembersArray[0].port   = prvTgfPortsArray[PRV_TGF_FDB_PORT_0_IDX_CNS];
    enabledMembersArray[1].hwDevice = prvTgfDevsArray[PRV_TGF_FDB_PORT_3_IDX_CNS];
    enabledMembersArray[1].port   = prvTgfPortsArray[PRV_TGF_FDB_PORT_3_IDX_CNS];

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    rc = prvTgfTrunkMembersSet(trunkId /*trunkId*/,
                    2 /*numOfEnabledMembers*/,
                    enabledMembersArray, 0 /*numOfDisabledMembers*/,
                    (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet");
}

/**
* @internal prvTgfOamBridgeConfigurationRestore function
* @endinternal
*
* @brief   Restore OAM test bridge configuration
*/
GT_VOID prvTgfOamBridgeConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_TRUNK_ID trunkId;            /* trunk Id */

    /* Invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfBrgDefVlanEntryInvalidate: %d",
                    PRV_TGF_VLANID_CNS);

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    /* Delete trunk */
    rc = prvTgfTrunkMembersSet(
                    trunkId /*trunkId*/,
                    0 /*numOfEnabledMembers*/,
                    (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
                    0 /*numOfDisabledMembers*/,
                    (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

/**
* @internal prvTgfOamIPclConfigurationRestore function
* @endinternal
*
* @brief   Restore OAM test PCL configuration
*/
static GT_VOID prvTgfOamIPclConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      ruleIndex;                      /* TCAM rule index */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr; /* Billing Counters */

    /* set CFM ethertype */
    rc = prvTgfVntCfmEtherTypeSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
#if 0
    /* set CFM ethertype UDE0 */
    rc = prvTgfPclUdeEtherTypeSet(0,/* UDE0 */ TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
#endif /*0*/
    /* UDB3 - MEG Level */
    rc = prvTgfPclUserDefinedByteSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    3 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E,
                    0/*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", 3);

    /* UDB2 - OAM opcode */
    rc = prvTgfPclUserDefinedByteSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    2 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E,
                    0/*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 2);

    /* UDB5 - OAM RDI */
    rc = prvTgfPclUserDefinedByteSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    5 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E,
                    0/*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 5);

    /* UDB7 - OAM Tx Period */
    rc = prvTgfPclUserDefinedByteSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    7 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E,
                    0/*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 7);

    /* PRV_TGF_PCL_UDE_UDB_INDEX_CNS - MEG Level */
    rc = prvTgfPclUserDefinedByteSet(
                    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    PRV_TGF_PCL_UDE_UDB_INDEX_CNS /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E,
                    0/*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", 15);

    for(ruleIndex = 0; ruleIndex < PRV_TGF_IOAM_PCL_RULE_NUM_STC; ruleIndex++)
    {
        /* invalidate PCL rules */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                    ruleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclRuleValidStatusSet: %d", GT_FALSE);

        cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

        oamConfig.oamEnable = GT_FALSE;
        oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS + ruleIndex;
        /* Reset common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);

        /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
        prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);

        /* Reset IPLR billing entries */
        cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            prvTgfBillingCntr.packetSizeMode =
                CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        }
        rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS + ruleIndex,
                    &prvTgfBillingCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);
    }

    /* restore PCL configuration table */
    prvTgfPclPortsRestoreAll();

    /* Set global stage mode for IPLR0 */
    rc = prvTgfPolicerStageMeterModeSet(
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet");

    /* Reset policer counting mode */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    PRV_TGF_POLICER_COUNTING_DISABLE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet");

    /* Reset hash bit selection to register default value */
    rc = prvTgfOamHashBitSelectionSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, 0, 11);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfOamHashBitSelectionSet");
}

/**
* @internal prvTgfOamEPclConfigurationRestore function
* @endinternal
*
* @brief   Restore egress OAM test PCL configuration
*/
static GT_VOID prvTgfOamEPclConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;
    GT_U32                      ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpRuleFormat;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;
    PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr; /* Billing Counters */
    GT_U32                      portIndex;

    nonIpRuleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;


    /* UDB3 - MEG Level */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 3 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 3);

    /* UDB2 - OAM opcode */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 1 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 1);

    /* UDB6 - OAM RDI */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 5 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", 5);

    /* UDB7 - OAM Tx Period */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 7 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E, 0 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 7);

    for(ruleIndex = 0; ruleIndex < oamEPclRulesNum; ruleIndex++)
    {
        /* clear PCL configuration table */
        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
        cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));

        interfaceInfo.type               = oamEPclRuleInfo[ruleIndex].type;
        interfaceInfo.devPort.hwDevNum     =
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk];
        interfaceInfo.devPort.portNum    =
                    prvTgfPortsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk];

        lookupCfg.pclId                  = PRV_TGF_EPCL_PCL_ID_0_CNS + ruleIndex;
        lookupCfg.enableLookup           = GT_FALSE;
        lookupCfg.dualLookup             = GT_FALSE;
        lookupCfg.groupKeyTypes.nonIpKey =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
        lookupCfg.groupKeyTypes.ipv6Key =
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

        rc = prvTgfPclCfgTblSet(
                    &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                    CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

        /* invalidate PCL rules */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E,
                    prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ruleIndex), GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPclRuleValidStatusSet: %d, %d",
                    prvTgfDevNum, GT_TRUE);
        /* used UDB0-0 that not muxed with other fields */
        rc = prvTgfPclUserDefinedByteSet(
                    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E,
                    0 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_INVALID_E,
                    0/*offset*/);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: index %d, offType %d",
                    0, PRV_TGF_PCL_OFFSET_INVALID_E);

        /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
        prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);

        /* Reset EPLR billing entries */
        cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            prvTgfBillingCntr.packetSizeMode =
                CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        }
        rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_EGRESS_E,
                    POLICER_MASK(PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS + ruleIndex),
                    &prvTgfBillingCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);

        cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

        oamConfig.oamEntryIndex = PRV_EOAM_DM_ENTRY_START_INDEX_CNS + ruleIndex;
        /* Reset common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);
    }

    for(portIndex = 0; (portIndex < PRV_TGF_EPCL_PORTS_NUM_CNS); portIndex++)
    {
        prvTgfPortsArray[portIndex] = epclPortsArray[portIndex];
    }

    /* AUTODOC: Clear EPCL Engine configuration tables */
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E);

    /* Reset hash bit selection to register default value */
    rc = prvTgfOamHashBitSelectionSet(
                    PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, 0, 11);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfOamHashBitSelectionSet");

    /* Restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/**
* @internal prvTgfOamIPclConfigurationSet function
* @endinternal
*
* @brief   Set OAM test Ingress PCL configuration and OAM related info
*
* @note Create 4 TCAM rules:
*       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
*       trigger Vlan Id = 5, source port 1, ethertype 0x8902; MEL 3
*       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
*       trigger Vlan Id = 5, source port 1, ethertype 0x8902; MEL - Don't care
*       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
*       trigger Vlan Id = 5, Trunk Id = 6, MEL 5
*       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
*       trigger Vlan Id = 5, source port 2, ethertype 0x8902; MEL 3
*
*/
static GT_VOID prvTgfOamIPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpRuleFormat; /* TCAM rule format */
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;           /* TCAM rule mask */
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;        /* TCAM rule data */
    PRV_TGF_PCL_ACTION_STC      action;         /* PCL action */
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;      /* lookup configuration */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration */
    GT_U32 ruleIndex;                           /* TCAM rule index */
    PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr; /* Billing Counters */

    nonIpRuleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* set CFM ethertype */
    rc = prvTgfVntCfmEtherTypeSet(TGF_ETHERTYPE_8902_CFM_TAG_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
#if 0
    /* set CFM ethertype UDE0 */
    rc = prvTgfPclUdeEtherTypeSet(
                    0,/* UDE0 */ TGF_ETHERTYPE_8902_CFM_TAG_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
#endif /*0*/
    /* UDB3 - MEG Level */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E, 3 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 2 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 3);

    /* UDB1 - OAM opcode */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E, 1 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 3 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 1);

    /* UDB5 - OAM RDI */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E, 5 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 4 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 5);

    /* UDB7 - OAM Tx Period */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E, 7 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 4 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 7);

    /* PRV_TGF_PCL_UDE_UDB_INDEX_CNS - MEG Level */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    PRV_TGF_PCL_UDE_UDB_INDEX_CNS /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 2 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", 15);

    /* Set OAM UDB profile 1 default value and mask */
    rc = prvTgfPclOamRdiSet(CPSS_PCL_DIRECTION_INGRESS_E,
                    PRV_TGF_OAM_PROFILE_INDEX_CNS,
                    PRV_TGF_OAM_RDI_UDB_VALUE_STC,
                    PRV_TGF_OAM_RDI_PROFILE_1_UDB_MASK_STC);


    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    lookupCfg.enableLookup = GT_TRUE;

    lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* init PCL Engine for send port */
    lookupCfg.pclId = PRV_TGF_SEND_PORT_IDX_CNS;
    rc = prvTgfPclDefPortInitExt2(
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPclDefPortInitExt2: %d",
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* init PCL Engine for recieve port */
    lookupCfg.pclId = PRV_TGF_RECEIVE_PORT_IDX_CNS;
    rc = prvTgfPclDefPortInitExt2(
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPclDefPortInitExt2: %d",
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* init PCL Engine for trunk port */
    lookupCfg.pclId = PRV_TGF_FDB_PORT_0_IDX_CNS;
    rc = prvTgfPclDefPortInitExt2(
                prvTgfPortsArray[PRV_TGF_FDB_PORT_0_IDX_CNS],
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPclDefPortInitExt2: %d",
                prvTgfPortsArray[PRV_TGF_FDB_PORT_0_IDX_CNS]);

    for(ruleIndex = 0; ruleIndex < PRV_TGF_IOAM_PCL_RULE_NUM_STC; ruleIndex++)
    {
        /* clear mask, pattern and action */
        cpssOsMemSet(&mask,   0, sizeof(mask));
        cpssOsMemSet(&pattern,0, sizeof(pattern));
        cpssOsMemSet(&action, 0, sizeof(action));

        /* PCL ID */
        mask.ruleStdNotIp.common.pclId = 0x3FF;
        pattern.ruleStdNotIp.common.pclId =
                (GT_U16)oamIPclRuleInfo[ruleIndex].srcPortTrunk;

        if (oamIPclRuleInfo[ruleIndex].pduHeaderByteValue !=
                    PRV_TGF_MEL_BYTE_DONT_CARE_CNS)
        {
            /* MEG Level (lookup 4-th byte in MAC SA) */
            mask.ruleStdNotIp.macSa.arEther[PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC] = 0xFF;
            pattern.ruleStdNotIp.macSa.arEther[PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC] =
                    oamIPclRuleInfo[ruleIndex].pduHeaderByteValue;
        }

        /* Ethertype */
        mask.ruleStdNotIp.etherType = 0xFFFF;
        pattern.ruleStdNotIp.etherType = TGF_ETHERTYPE_8902_CFM_TAG_CNS;

        /* Set action */
        action.egressPolicy = GT_FALSE;
        action.pktCmd =
                    CPSS_PACKET_CMD_FORWARD_E;

        if(testUseTtiForOam == GT_FALSE)
        {
            action.oam.oamProcessEnable = GT_TRUE;
            action.oam.oamProfile =
                        PRV_TGF_OAM_PROFILE_INDEX_CNS;
        }

        action.flowId =
                    PRV_TGF_PCL_ACTION_FLOW_ID_CNS + ruleIndex;
        action.policer.policerEnable =
                    PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        action.policer.policerId =
                    PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS + ruleIndex;

        /* Set PCL rule */
        rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                    PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS + (ruleIndex * 3),
                    &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                    "prvTgfPclRuleSet: %d, %d, %d",
                    prvTgfDevNum, PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                    PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS);

        /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
        prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_INGRESS_0_E);

        /* Set IPLR billing entries */
        cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
        prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            prvTgfBillingCntr.lmCntrCaptureMode =
                PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_COPY_E;
            prvTgfBillingCntr.packetSizeMode =
                CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        }

        rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS + ruleIndex,
                    &prvTgfBillingCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);

        /* Set OAM configuration  */
        cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

        /* MEL = 3 */
        oamConfig.oamEnable = GT_TRUE;
        oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS + ruleIndex;
        oamConfig.oamEntry.sourceInterface.type = oamIPclRuleInfo[ruleIndex].type;
        if(oamConfig.oamEntry.sourceInterface.type == CPSS_INTERFACE_PORT_E)
        {
            /* Source interface - device/port */
            oamConfig.oamEntry.sourceInterface.devPort.hwDevNum =
                    prvTgfDevsArray[oamIPclRuleInfo[ruleIndex].srcPortTrunk];
            oamConfig.oamEntry.sourceInterface.devPort.portNum =
                    prvTgfPortsArray[oamIPclRuleInfo[ruleIndex].srcPortTrunk];
        }
        else
        {
            /* Source interface - trunk */
            oamConfig.oamEntry.sourceInterface.trunkId = PRV_TGF_TRUNK_ID_CNS;
        }
        oamConfig.oamEntry.megLevelCheckEnable = GT_FALSE;
        oamConfig.oamEntry.megLevel = 3;
        oamConfig.oamExceptionType = PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
        oamConfig.oamExceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        oamConfig.oamExceptionConfig.cpuCode =
                    CPSS_NET_FIRST_USER_DEFINED_E +
                    PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
        oamConfig.oamExceptionConfig.summaryBitEnable = GT_TRUE;
        if (oamIPclRuleInfo[ruleIndex].pduHeaderByteValue == PRV_TGF_MEL_BYTE_DONT_CARE_CNS)
        {
            /* MEL = 5  */
            oamConfig.oamEntry.megLevelCheckEnable = GT_TRUE;
            oamConfig.oamEntry.megLevel = 5;
        }
        /* Set common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);
    }

    /* Set policer stage to flow-based metering */
    rc = prvTgfPolicerStageMeterModeSet(
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerStageMeterModeSet: %d, %d",
                     PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                     PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);

    /* Set policer counting enable by Policy Action Entry*/
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet");

    /* Set LM counter capture enable */
    rc = prvTgfPolicerLossMeasurementCounterCaptureEnableSet(
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                    GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerLossMeasurementCounterCaptureEnableSet");

    /* Enable Billing for Trapped Packets */
    rc = prvTgfPolicerTrappedPacketsBillingEnableSet(
                    PRV_TGF_POLICER_STAGE_INGRESS_0_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerTrappedPacketsBillingEnableSet");

    /* Set default hash bit selection */
    rc = prvTgfOamHashBitSelectionSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, 0, 5);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfOamHashBitSelectionSet");
}


/**
* @internal prvTgfOamEPclConfigurationSet function
* @endinternal
*
* @brief   Set OAM test Egress PCL configuration and OAM related info
*
* @note Create 4 TCAM rules:
*       - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
*       trigger DSA Command = FROM_CPU0; Opcode = 46
*       - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
*       trigger DSA Command = TO_CPU1; Opcode = 46
*       - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
*       trigger DSA Command = FROM_CPU1; Opcode = 47
*       - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
*       trigger DSA Command = TO_CPU0; Opcode = 47
*
*/
static GT_VOID prvTgfOamEPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT nonIpRuleFormat;   /* TCAM rule format */
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;           /* TCAM rule mask */
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;        /* TCAM rule pattern */
    PRV_TGF_PCL_ACTION_STC      action;         /* PCL action */
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;      /* PCL lookup configuration */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration */
    GT_U32 ruleIndex;                           /* TCAM rule index */
    GT_U32 port;                                /* egress port */
    PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr; /* Billing Counters */

    /* Set egress ports array */
    for(port = 0; port < PRV_TGF_EPCL_PORTS_NUM_CNS - 1; port++)
    {
        /* Copy original ports */
        epclPortsArray[port] = prvTgfPortsArray[port];
    }
    /* Set CPU port */
    epclPortsArray[PRV_TGF_CPU_PORT_IDX_CNS] = PRV_CPSS_CPU_PORT_NUM_CNS;

    /* Set EPCL specific port array */
    rc = prvTgfDefPortsArraySet(epclPortsArray, PRV_TGF_EPCL_PORTS_NUM_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfDefPortsArraySet: %d", PRV_TGF_EPCL_PORTS_NUM_CNS);

    nonIpRuleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;


    /* UDB3 - MEG Level */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 3 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 2 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 3);

    /* UDB1 - OAM opcode */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 1 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 3 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 1);

    /* UDB5 - OAM RDI */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 5 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 4 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", 5);

    /* UDB7 - OAM Tx Period */
    rc = prvTgfPclUserDefinedByteSet(nonIpRuleFormat,
                    PRV_TGF_PCL_PACKET_TYPE_UDE_E,
                    CPSS_PCL_DIRECTION_EGRESS_E, 7 /*udbIndex*/,
                    PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, 4 /*offset*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclUserDefinedByteSet: %d", 7);


    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* Init EPCL Engine from CPU0 port */
    lookupCfg.pclId = PRV_TGF_EPCL_PCL_ID_0_CNS + PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS;
    rc = prvTgfPclDefPortInitExt2(
                prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
                CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInitExt2: %d",
                prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);


    /* Init EPCL Engine from CPU1 port */
    lookupCfg.pclId = PRV_TGF_EPCL_PCL_ID_0_CNS + PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS;
    rc = prvTgfPclDefPortInitExt2(
                prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS],
                CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInitExt2: %d",
                prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS]);

    /* Init EPCL Engine to CPU port */
    lookupCfg.pclId = PRV_TGF_EPCL_PCL_ID_0_CNS + PRV_TGF_CPU_PORT_IDX_CNS;
    rc = prvTgfPclDefPortInitExt2(
                prvTgfPortsArray[PRV_TGF_CPU_PORT_IDX_CNS],
                CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInitExt2: %d",
                prvTgfPortsArray[PRV_TGF_CPU_PORT_IDX_CNS]);

    for(ruleIndex = 0; ruleIndex < oamEPclRulesNum; ruleIndex++)
    {
        /* Set DSA Tag Commands on egress ports:
                    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,
                    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E,
                    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E
        */

        /* clear mask, pattern and action */
        cpssOsMemSet(&mask,   0, sizeof(mask));
        cpssOsMemSet(&pattern,0, sizeof(pattern));
        cpssOsMemSet(&action, 0, sizeof(action));

        rc = prvTgfPclEgressPclPacketTypesSet(
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    prvTgfPortsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_NON_TS_E,
                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPclEgressPclPacketTypesSet: %d, %d",
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_NON_TS_E);

        switch(oamEPclRuleInfo[ruleIndex].pktType)
        {
            case PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E:
            case PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E:
                rc = prvTgfPclEgressPclPacketTypesSet(
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    prvTgfPortsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,
                    GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPclEgressPclPacketTypesSet: %d, %d",
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E);

                rc = prvTgfPclEgressPclPacketTypesSet(
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    prvTgfPortsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E,
                    GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPclEgressPclPacketTypesSet: %d, %d",
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_DATA_E);

                mask.ruleEgrStdNotIp.common.egrPacketType = 0x3;
                pattern.ruleEgrStdNotIp.common.egrPacketType =
                    PRV_TGF_PKT_CMD_FROM_CPU_CNS;
                break;
            case PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E:
            default:
                rc = prvTgfPclEgressPclPacketTypesSet(
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    prvTgfPortsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E,
                    GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPclEgressPclPacketTypesSet: %d, %d",
                    prvTgfDevsArray[oamEPclRuleInfo[ruleIndex].trgPortTrunk],
                    PRV_TGF_PCL_EGRESS_PKT_TO_CPU_E);

                mask.ruleEgrStdNotIp.common.egrPacketType = 0x3;
                pattern.ruleEgrStdNotIp.common.egrPacketType = PRV_TGF_PKT_CMD_TO_CPU_CNS;
                break;
        }

        /* PCL ID */
        mask.ruleEgrStdNotIp.common.pclId = 0x3FF;
        pattern.ruleEgrStdNotIp.common.pclId =
                    (GT_U16)(PRV_TGF_EPCL_PCL_ID_0_CNS + oamEPclRuleInfo[ruleIndex].trgPortTrunk);


        /* OAM Opcode (lookup 4-th byte in MAC SA) */
        mask.ruleEgrStdNotIp.macSa.arEther[PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC] = 0xFF;
        pattern.ruleEgrStdNotIp.macSa.arEther[PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC] =
                    oamEPclRuleInfo[ruleIndex].pduHeaderByteValue;

        /* Ethertype */
        mask.ruleEgrStdNotIp.etherType = 0xFFFF;
        pattern.ruleEgrStdNotIp.etherType = TGF_ETHERTYPE_8902_CFM_TAG_CNS;

        /* Set action */
        action.egressPolicy = GT_TRUE;
        action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        action.oam.oamProcessEnable = GT_TRUE;
        action.oam.oamProfile = PRV_TGF_OAM_PROFILE_INDEX_CNS;
        action.oam.timeStampEnable =
                    (ruleIndex < PRV_TGF_EOAM_DM_PCL_RULE_NUM_STC) ?
                    GT_TRUE : GT_FALSE;
        action.oam.offsetIndex = ruleIndex;
        action.flowId = PRV_TGF_EPCL_ACTION_FLOW_ID_CNS + ruleIndex;
        action.policer.policerEnable =
                    PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        action.policer.policerId =
                    POLICER_MASK(PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS + ruleIndex);


        /* Set PCL rule */
        rc = prvTgfPclRuleSet(nonIpRuleFormat,
                    PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS + (ruleIndex * 3),
                    &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                    "prvTgfPclRuleSet: %d, %d, %d",
                    prvTgfDevNum, nonIpRuleFormat,
                    PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS);

        /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
        prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);

        /* Set EPLR billing entries */
        cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
        prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            prvTgfBillingCntr.lmCntrCaptureMode =
                PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E;
            prvTgfBillingCntr.packetSizeMode =
                CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        }

        rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_EGRESS_E,
                    POLICER_MASK(PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS + ruleIndex),
                    &prvTgfBillingCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);

        /* Set OAM configuration  */
        cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

        oamConfig.oamEnable = GT_TRUE;
        oamConfig.oamEntryIndex = PRV_EOAM_DM_ENTRY_START_INDEX_CNS + ruleIndex;
        /* Set common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);
    }

    /* Set LM counter capture enable */
    rc = prvTgfPolicerLossMeasurementCounterCaptureEnableSet(
                    PRV_TGF_POLICER_STAGE_EGRESS_E,
                    GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerLossMeasurementCounterCaptureEnableSet");

    /* Enable Billing for Trapped Packets */
    rc = prvTgfPolicerTrappedPacketsBillingEnableSet(
                    PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerTrappedPacketsBillingEnableSet");

    /* Set default hash bit selection */
    rc = prvTgfOamHashBitSelectionSet(
                    PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, 0, 5);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "prvTgfOamHashBitSelectionSet");

}

/**
* @internal prvTgfOamConfigurationSet function
* @endinternal
*
* @brief   Set OAM common configuration
*
* @param[in] oamConfigPtr             - pointer to common OAM configuration
*                                       None
*/
GT_VOID prvTgfOamConfigurationSet
(
    IN PRV_TGF_OAM_COMMON_CONFIG_STC * oamConfigPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_OAM_STAGE_TYPE_ENT stage =
        (oamConfigPtr->oamEntryIndex < PRV_EOAM_DM_ENTRY_START_INDEX_CNS) ?
        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E : PRV_TGF_OAM_STAGE_TYPE_EGRESS_E;

    if(oamConfigPtr->oamEnable)
    {
        /* enable OAM processing */
        rc = prvTgfOamEnableSet(stage, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamEnableSet");

        /* set OAM flow Id base */
        rc = prvTgfOamTableBaseFlowIdSet(stage, PRV_TGF_BASEFLOW_ID_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");
    }
    else
    {
        /* disable OAM processing */
        rc = prvTgfOamEnableSet(stage, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamEnableSet");

        /* Reset OAM flow Id base */
        rc = prvTgfOamTableBaseFlowIdSet(stage, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamTableBaseFlowIdSet");
    }

    /* set OAM entry */
    rc = prvTgfOamEntrySet( stage,
                    oamConfigPtr->oamEntryIndex,
                    &oamConfigPtr->oamEntry);

    UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamEntrySet");

    rc = prvTgfOamExceptionConfigSet(
                    stage, oamConfigPtr->oamExceptionType,
                    &oamConfigPtr->oamExceptionConfig);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamExceptionConfigSet");
}

/**
* @internal prvTgfOamConfigurationGet function
* @endinternal
*
* @brief   Get OAM common configuration
*
* @param[in] oamConfigPtr             - pointer to common OAM configuration
*
* @param[out] oamConfigPtr             - pointer to OAM entry.
*                                       None
*/
GT_VOID prvTgfOamConfigurationGet
(
    OUT PRV_TGF_OAM_COMMON_CONFIG_STC * oamConfigPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL enable;
    /* Get OAM processing status */
    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamEnableGet");

    if(enable)
    {
        /* Get OAM entry */
        rc = prvTgfOamEntryGet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamConfigPtr->oamEntryIndex,
                    &oamConfigPtr->oamEntry);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEntryGet");

        rc = prvTgfOamExceptionConfigGet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamConfigPtr->oamExceptionType,
                    &oamConfigPtr->oamExceptionConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
                    GT_OK, rc, "prvTgfOamExceptionConfigGet");
    }
    /* Set OAM processing status */
    oamConfigPtr->oamEnable = enable;
}

/**
* @internal prvTgfOamTtiConfigurationSet function
* @endinternal
*
* @brief   init/restore OAM test TTI configuration
*
* @param[in] doRestore                - GT_TRUE  - restore    configurations
*                                      GT_FALSE - initialize configurations
*                                       None
*/
static GT_VOID prvTgfOamTtiConfigurationSet
(
    IN GT_BOOL  doRestore
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ACTION_2_STC ttiAction2;
    GT_U32                   ttiIndex;
    GT_U32                   ingressPortsIndexesArr[]={
        PRV_TGF_SEND_PORT_IDX_CNS ,
        PRV_TGF_RECEIVE_PORT_IDX_CNS ,
        PRV_TGF_FDB_PORT_0_IDX_CNS,
        0xFFFFFFFF};
    GT_U32                      portIndex;
    PRV_TGF_TTI_KEY_TYPE_ENT    keyType = PRV_TGF_TTI_KEY_ETH_E;
    GT_BOOL     enable = (doRestore == GT_TRUE) ? GT_FALSE : GT_TRUE;
    GT_U32      ii;

    ttiIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 , 0x564 * 3);

    if(enable == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("======= Restore TTI Configuration =======\n");
    }

    for(ii = 0 ; ingressPortsIndexesArr[ii] < prvTgfPortsNum ; ii++)
    {
        portIndex = ingressPortsIndexesArr[ii];
        /* AUTODOC: enable/disable TTI lookup for relevant ports, key TTI_KEY_ETH */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[portIndex], keyType, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", 0);
    }

    if(enable == GT_TRUE)
    {
        /* AUTODOC: add TTI rule with: match all the packets */
        cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
        cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
        cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

        /* AUTODOC: set the TTI Rule Action */
        ttiAction2.tag0VlanCmd       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
        ttiAction2.tag0VlanId        = 0;

        ttiAction2.tag1VlanCmd = PRV_TGF_TTI_VLAN1_MODIFY_UNTAGGED_E;
        ttiAction2.tag1VlanId = 0;

        ttiAction2.oamProcessEnable                    = GT_TRUE;
        ttiAction2.oamProfile                          = PRV_TGF_OAM_PROFILE_INDEX_CNS;

        /* this should be overridden by the IPCL */
        ttiAction2.flowId                              = PRV_TGF_PCL_ACTION_FLOW_ID_CNS - 1;

        /* AUTODOC: set valid TTI rule */
        rc = prvTgfTtiRule2Set(ttiIndex, keyType, &ttiPattern, &ttiMask, &ttiAction2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: invalidate TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(ttiIndex, enable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, enable);
    }

}

/**
* @internal prvTgfDefOamGeneralConfigurationSet function
* @endinternal
*
* @brief   Set Default OAM general configuration.
*/
GT_VOID prvTgfDefOamGeneralConfigurationSet
(
    GT_VOID
)
{
    /* Restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();

    /* AUTODOC: Set common bridge configuration */
    prvTgfOamBridgeConfigurationSet();

    if(testUseTtiForOam == GT_TRUE)
    {
        /* AUTODOC: use TTI for enabling OAM processing and the 'oam profile' assignment */
        /* AUTODOC: note : in this mode still we use the IPCL for assignment of all other parameters */
        prvTgfOamTtiConfigurationSet(GT_FALSE/* initialize*/);
    }

    /* AUTODOC: Set common IPCL configuration */
    prvTgfOamIPclConfigurationSet();
    /* AUTODOC: Set egress PCL configuration */
    prvTgfOamEPclConfigurationSet();
}


/**
* @internal prvTgfDefOamGeneralConfigurationRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfDefOamGeneralConfigurationRestore
(
    GT_VOID
)
{
    /* AUTODOC: Restore common bridge configuration */
    prvTgfOamBridgeConfigurationRestore();
    if(testUseTtiForOam == GT_TRUE)
    {
        /* AUTODOC: Restore common TTI configurations */
        prvTgfOamTtiConfigurationSet(GT_TRUE/* restore */);
    }
    /* AUTODOC: Restore common IPCL configuration */
    prvTgfOamIPclConfigurationRestore();
    /* AUTODOC: Restore EPCL configuration */
    prvTgfOamEPclConfigurationRestore();
    /* AUTODOC: Restore original MAC addresses */
    prvTgfOamPacketHeaderDataReset();
}

/**
* @internal prvTgfDefOamGeneralTrafficCheck function
* @endinternal
*
* @brief   Check traffic results.
*
* @param[in] sendPort                 - send interface index
* @param[in] exceptionType            - exception type
* @param[in] expectedFlood            - flooded packets
* @param[in] expectedTrap             - trapped packets
* @param[in] expectedException        - number of exceptions
* @param[in] expectedBilling          - number of excpected billing counters
*                                       None
*/
GT_VOID prvTgfDefOamGeneralTrafficCheck
(
    IN GT_U32 sendPort,
    IN PRV_TGF_OAM_EXCEPTION_TYPE_ENT exceptionType,
    IN GT_U32 expectedFlood,
    IN GT_U32 expectedTrap,
    IN GT_U32 expectedException,
    IN GT_U32 expectedBilling
)
{
    GT_U32 sleepTime = 200;

    /* AUTODOC: Check packets flooded */
    prvTgfOamPortCountersForwardCheck(sendPort, expectedFlood);
    tgfSimSleep(sleepTime);

    /* AUTODOC: Check packets captured by CPU */
    prvTgfOamRxInCpuCheck(expectedTrap);
    tgfSimSleep(sleepTime);

    /* AUTODOC: Check exception */
    prvTgfOamExceptionTableCheck(exceptionType, expectedException);
    tgfSimSleep(sleepTime);

    /* AUTODOC: Check billing counters */
    prvTgfOamIngressBillingCounterCheck(expectedBilling);
    tgfSimSleep(sleepTime);
}

/**
* @internal prvTgfDefOamGeneralMelTrafficTest function
* @endinternal
*
* @brief   MEG level check
*/
GT_VOID prvTgfDefOamGeneralMelTrafficTest
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    counterValue;             /* counter value */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData;  /* PDU header data */

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* reset meg level counter ... that we will later check */
    /* without it run after other test may fail us */
    rc = prvTgfOamExceptionCounterGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E, &counterValue);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                    "prvTgfOamExceptionCounterGet: %d, %d",
                    prvTgfDevNum,
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E);

    /* AUTODOC: Set MEG level = 3 */
    pduHeaderData.megLevel = 3;
    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                    prvTgfOamBurstCount, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0  /* Billing counters */);
    /* AUTODOC: Set OAM PDU header MEL = 0, 1, 2, 4 */
    for(pduHeaderData.megLevel = 0; pduHeaderData.megLevel < 5; pduHeaderData.megLevel++)
    {
        if(pduHeaderData.megLevel == 3)
        {
            continue;
        }

        /* AUTODOC: Set source mac address byte */
        prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);
        prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);
    }

    /* AUTODOC: Set OAM PDU header MEL = 5, 6, 7 */
    for(pduHeaderData.megLevel = 5; pduHeaderData.megLevel < 8; pduHeaderData.megLevel++)
    {
        /* AUTODOC: Set source mac address byte */
        prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);
        prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Forwarded */
        prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E,
                    prvTgfOamBurstCount, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0  /* Billing counters */);
    }
}

/**
* @internal prvTgfDefOamGeneralSourceInterfaceTest function
* @endinternal
*
* @brief   Source Interface Verification
*/
GT_VOID prvTgfDefOamGeneralSourceInterfaceTest
(
    GT_VOID
)
{

    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

    oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS;

    /* Get OAM configuration*/
    prvTgfOamConfigurationGet(&oamConfig);

    /* AUTODOC: Source Interface NO Match/Source Interface Port */
    oamConfig.oamEntry.sourceInterfaceCheckEnable = GT_TRUE;
    oamConfig.oamEntry.sourceInterfaceCheckMode =
                    PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E;
    oamConfig.oamExceptionType = PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E;
    oamConfig.oamExceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    oamConfig.oamExceptionConfig.cpuCode =
                    CPSS_NET_FIRST_USER_DEFINED_E +
                    PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E;
    oamConfig.oamExceptionConfig.summaryBitEnable = GT_TRUE;

    /* AUTODOC: Set OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set MEL = 3 */
    pduHeaderData.megLevel = 3;
    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                     PRV_TGF_SEND_PORT_IDX_CNS,
                     PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
                     0, /* Number of sent packets */
                     prvTgfOamBurstCount, /* Packets trapped to CPU */
                     prvTgfOamBurstCount, /* Exceptions */
                     0 /* Billing counters */);

    /* AUTODOC: Source Interface Match/Source Interface Port */
    oamConfig.oamEntry.sourceInterfaceCheckMode =
        PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_MATCH_E;

    /* AUTODOC: Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
                    prvTgfOamBurstCount, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0  /* Billing counters */);

    /* AUTODOC: Source Interface Match/Source Interface Trunk */
    oamConfig.oamEntryIndex = PRV_OAM_ENTRY_SRC_TRUNK_INDEX_CNS;
    /* Get OAM common configuration */
    prvTgfOamConfigurationGet(&oamConfig);

    /* AUTODOC: Source Interface NO Match/Source Interface Port */
    oamConfig.oamEntry.megLevel = 5;
    oamConfig.oamEntry.sourceInterfaceCheckEnable = GT_TRUE;
    oamConfig.oamEntry.sourceInterfaceCheckMode =
        PRV_TGF_OAM_SOURCE_INTERFACE_CHECK_MODE_NO_MATCH_E;
    oamConfig.oamExceptionType = PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E;
    oamConfig.oamExceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    oamConfig.oamExceptionConfig.cpuCode =
        CPSS_NET_FIRST_USER_DEFINED_E + PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E;
    oamConfig.oamExceptionConfig.summaryBitEnable = GT_TRUE;

    /* AUTODOC: Set OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set OAM PDU header MEL = 5*/
    pduHeaderData.megLevel = 5;
    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FDB_PORT_0_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FDB_PORT_0_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_SOURCE_INTERFACE_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);
}

/**
* @internal prvTgfDefOamGeneralDualEndedLossMeasurementTest function
* @endinternal
*
* @brief   Dual-Ended Loss Measurement Packets Test
*/
GT_VOID prvTgfDefOamGeneralDualEndedLossMeasurementTest
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8 opcode;                               /* Opcode value */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

    oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS;
    /* Get OAM configuration */
    prvTgfOamConfigurationGet(&oamConfig);

    oamConfig.oamEntry.opcodeParsingEnable = GT_TRUE;
    oamConfig.oamEntry.dualEndedLmEnable = GT_TRUE;

    /* AUTODOC: Set OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set dual-ended opcode */
    opcode = PRV_TGF_OPCODE_BASE_VALUE_CNS + PRV_TGF_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E;

    rc = prvTgfOamOpcodeSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E,
                    0, opcode);

    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamOpcodeSet");
    /* AUTODOC: Set packet command trap to CPU for dual-ended opcode OAM packets */
    rc = prvTgfOamDualEndedLmPacketCommandSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    CPSS_PACKET_CMD_TRAP_TO_CPU_E, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamDualEndedLmPacketCommandSet");

    /* AUTODOC: Set CPU code base */
    rc = prvTgfOamCpuCodeBaseSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    CPSS_NET_FIRST_USER_DEFINED_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamCpuCodeBaseSet");

    /* AUTODOC: Set PDU header: MEL = 3, dual-ended opcode */
    pduHeaderData.megLevel = 3;
    /* Set OAM Opcode */
    pduHeaderData.opcode = opcode;
    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);
    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);
}

/**
* @internal prvTgfOamPacketHeaderMacSwap function
* @endinternal
*
* @brief   Swap MAC addresses in packet header
*/
GT_VOID prvTgfOamPacketHeaderMacSwap
(
     GT_VOID
)
{
    TGF_MAC_ADDR    saMac;                  /* MAC address */

    cpssOsMemCpy(&saMac, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, &saMac, sizeof(TGF_MAC_ADDR));
}

/**
* @internal prvTgfOamPacketHeaderSrcMacByteSet function
* @endinternal
*
* @brief   Set source MAC addresses in packet header
*/
GT_VOID prvTgfOamPacketHeaderSrcMacByteSet
(
     GT_U32     byteIndex,
     GT_U8      byteValue
)
{
    TGF_PACKET_L2_STC * tgfPartL2Ptr =
        (TGF_PACKET_L2_STC *)prvTgfPacketInfo.partsArray[0].partPtr;
    tgfPartL2Ptr->saMac[byteIndex] = byteValue;
}

/**
* @internal prvTgfOamGenericUseTtiForOam function
* @endinternal
*
* @brief   Set mode for the test: using TTI/IPCL for the OAM profile and flowId assignment
*         in any case the IPCL is used for other parameters setting.
* @param[in] useTtiForOam             - GT_TRUE - use TTI  for the OAM profile and flowId assignment
*                                      GT_TRUE - use IPCL for the OAM profile and flowId assignment
*                                       None
*/
GT_VOID prvTgfOamGenericUseTtiForOam
(
    IN GT_BOOL  useTtiForOam
)
{
    testUseTtiForOam = useTtiForOam;
}


