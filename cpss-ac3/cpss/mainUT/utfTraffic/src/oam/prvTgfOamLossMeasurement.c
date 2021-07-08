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
* @file prvTgfOamLossMeasurement.c
*
* @brief OAM Loss Measurement features testing
*
* @version   11
********************************************************************************
*/
#include <oam/prvTgfOamLossMeasurement.h>

/* OAM EPCL Lookup Configuration */
extern PRV_TGF_OAM_EPCL_RULE_CONFIG_STC oamEPclRuleInfo[];
extern TGF_PACKET_STC * capturePacketsArray[];

/* OAM packet command profile */
#define PRV_TGF_OAM_PACKET_CMD_PROFILE_STC              3

/* IPCL configuration index for send/recieve port of CPU0 */
#define PRV_TGF_OAM_IPCL_LM_RULE_PP0_PORT_INDEX_STC     0
/* IPCL configuration index for send/recieve port of CPU1 */
#define PRV_TGF_OAM_IPCL_LM_RULE_PP1_PORT_INDEX_STC     4

/* EPCL configuration index for send/recieve port of CPU0 */
#define PRV_TGF_OAM_EPCL_LM_RULE_CPU0_PORT_INDEX_STC    4
/* EPCL configuration index for send/recieve port of CPU1 */
#define PRV_TGF_OAM_EPCL_LM_RULE_CPU1_PORT_INDEX_STC    6

/* EPCL configuration index for send/recieve port of PP0 */
#define PRV_TGF_OAM_EPCL_LM_RULE_PP0_PORT_INDEX_STC     5
/* EPCL configuration index for send/recieve port of PP1 */
#define PRV_TGF_OAM_EPCL_LM_RULE_PP1_PORT_INDEX_STC     7

/* Unknown ethertype */
#define TGF_ETHERTYPE_5555_CFM_TAG_CNS                  0x5555

/* Burst packets number */
extern GT_U32 prvTgfOamBurstCount;

/* L2 part of broadcast packet */
static TGF_PACKET_L2_STC prvTgfBroadcastPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};

/* Unknown packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketUnknownEtherTypePart = {
    TGF_ETHERTYPE_5555_CFM_TAG_CNS};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal tgfOamLossMeasurementTest function
* @endinternal
*
* @brief   Loss Measurement Verification
*/
GT_VOID tgfOamLossMeasurementTest
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;         /* return code */
    GT_U32          oamIndex;           /* OAM entry index */
    GT_U32          opCodeIndex;        /* opcode index range: 0..15 */
    GT_U32          opCode;             /* opcode value */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */
    GT_U8           *tlvEntryPtr;       /* pointer to TLV data */
    GT_U32          txPp0Counter[2];    /* PP0 Tx counters */
    GT_U32          rxPp0Counter[2];    /* PP0 Rx counters */
    GT_U32          txPp1Counter[2];    /* PP1 Tx counters */
    GT_U32          rxPp1Counter[2];    /* PP1 Rx counters */
    GT_U32          packetToSend;       /* number of packet to send */
    GT_U32          lmResult;           /* loss measurement result */
    PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr; /* Billing Counters */

    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* AUTODOC: Set LMM/LMR opcodes */
    for(opCodeIndex = 0; opCodeIndex < 2; opCodeIndex++)
    {
        switch(opCodeIndex)
        {
            case 0:
                /* Set OAM LM0 Opcode */
                opCode = PRV_TGF_LM_OPCODE_BYTE_42_CNS;
                break;
            default:
                /* Set OAM LM1 Opcode */
                opCode = PRV_TGF_LM_OPCODE_BYTE_43_CNS;
                break;
        }

        rc = prvTgfOamOpcodeSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_OPCODE_TYPE_LM_SINGLE_ENDED_E,
                    opCodeIndex, opCode);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeSet");
    }

    /* AUTODOC: Set IOAM configuration */
    for(oamIndex = 0; oamIndex < PRV_TGF_IOAM_PCL_RULE_NUM_STC; oamIndex++)
    {

        oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS + oamIndex;
        /* Get OAM common configuration */
        prvTgfOamConfigurationGet(&oamConfig);

        /* Set MEG level */
        oamConfig.oamEntry.megLevel = 5;

        /* Enable LM counting and capture */
        oamConfig.oamEntry.lmCountingMode = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
        oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;
        /* Enable Opcode parsing */
        oamConfig.oamEntry.opcodeParsingEnable = GT_TRUE;
        oamConfig.oamEntry.packetCommandProfile =
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC;

        /* Set common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);
    }

    /* AUTODOC: Set EOAM configuration */
    for(oamIndex = 0; oamIndex < PRV_TGF_EOAM_LM_PCL_RULE_NUM_STC; oamIndex++)
    {
        oamConfig.oamEntryIndex =
                    PRV_EOAM_LM_ENTRY_START_INDEX_CNS + oamIndex;

        /* Get OAM common configuration */
        prvTgfOamConfigurationGet(&oamConfig);

        oamConfig.oamEntry.oamPtpOffsetIndex = oamIndex;
        oamConfig.oamEntry.opcodeParsingEnable = GT_FALSE;
        /* Enable LM counting and capture */
        oamConfig.oamEntry.lmCountingMode = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
        oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;

        /* Set common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);
        /* Set OAM LM offset table */
        prvTgfOamLmOffsetTableSet(oamIndex,
                    PRV_TGF_OAM_TLV_DATA_OFFSET_STC + 4 * oamIndex);
    }

    /* AUTODOC: Set LMM Opcode/Profile Packet Command for IOAM packets */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamEPclRuleInfo[PRV_TGF_OAM_EPCL_LM_RULE_PP0_PORT_INDEX_STC].pduHeaderByteValue,
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC,
                    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                    2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeProfilePacketCommandEntrySet");

    /* AUTODOC: Set LMR Opcode/Profile Packet Command for IOAM packets */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamEPclRuleInfo[PRV_TGF_OAM_EPCL_LM_RULE_PP1_PORT_INDEX_STC].pduHeaderByteValue,
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC,
                    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                    2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeProfilePacketCommandEntrySet");
    /* AUTODOC: Set OAM CPU code base fro DMM/DMR packets trapped to CPU */
    rc = prvTgfOamCpuCodeBaseSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    CPSS_NET_FIRST_USER_DEFINED_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamCpuCodeBaseSet");

    PRV_UTF_LOG0_MAC("======= Send burst of 20 not OAM broadcast packets =======\n");


    /* Set L2 broadcast DA */
    prvTgfOamPacketHeaderDataSet(TGF_PACKET_PART_L2_E,
                    &prvTgfBroadcastPacketL2Part);
    /* Set unknown ethertype for broadcast traffic */
    prvTgfOamPacketHeaderDataSet(TGF_PACKET_PART_ETHERTYPE_E,
                    &prvTgfPacketUnknownEtherTypePart);

    /* AUTODOC: Send burst of 20 broadcast packets to port 1 */
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    20,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    20, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0 /* Billing counters */);

    /* AUTODOC: Restore original MAC addresses */
    prvTgfOamPacketHeaderDataReset();
    /* Restore to default burst */
    prvTgfOamBurstCount = PRV_TGF_OAM_BURST_COUNT_CNS;

    PRV_UTF_LOG0_MAC("======= Single-ended LM protocol =======\n");

    /* AUTODOC: Single-ended LM protocol */
    for(packetToSend = 0; packetToSend < 2; packetToSend++)
    {

        /* AUTODOC: Phase 1: Send LMM packet from "CPU0" to "CPU1" */

        /* Set MEL = 5 */
        pduHeaderData.megLevel = 5;
        /* Set TLV offset */
        pduHeaderData.tlvOffset = 16;

        /* Set LMM OAM Opcode - CPU0*/
        pduHeaderData.opcode = PRV_TGF_LM_OPCODE_BYTE_42_CNS;

        /* AUTODOC: Set source mac address byte */
        prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.opcode);
        prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

        /* Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

        /* Parse from CPU0 OAM PDU header - LMM */
        prvTgfOamRxPduHeaderParse(capturePacketsArray[0], &pduHeaderData);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    5, pduHeaderData.megLevel, "Form CPU0 MEL Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_LM_OPCODE_BYTE_42_CNS, pduHeaderData.opcode, "From CPU0 Opcode Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);

        /* Phase 2: Send LMR packet from "CPU1" to "CPU0" */

        /* AUTODOC: Swap MAC addresses */
        prvTgfOamPacketHeaderMacSwap();

        /* AUTODOC: Set LMR OAM Opcode - CPU1*/
        pduHeaderData.opcode = PRV_TGF_LM_OPCODE_BYTE_43_CNS;
        /* AUTODOC: Set source mac address byte */
        prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.opcode);
        prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

        /* Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

        /* Parse from CPU0 OAM PDU header - LMR */
        prvTgfOamRxPduHeaderParse(capturePacketsArray[0], &pduHeaderData);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    5, pduHeaderData.megLevel, "Form CPU1 MEL Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_LM_OPCODE_BYTE_43_CNS, pduHeaderData.opcode, "From CPU1 Opcode Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS]);

        tlvEntryPtr = &pduHeaderData.tlvDataArr[0];

        /* Get counters for the first LMM/LMR packet */
        txPp0Counter[packetToSend] =
                    tlvEntryPtr[0] << 24 | tlvEntryPtr[1] << 16 |
                    tlvEntryPtr[2] << 8 | tlvEntryPtr[3];
        rxPp1Counter[packetToSend] =
                    tlvEntryPtr[4] << 24 | tlvEntryPtr[5] << 16 |
                    tlvEntryPtr[6] << 8 | tlvEntryPtr[7];
        txPp1Counter[packetToSend] =
                    tlvEntryPtr[8] << 24 | tlvEntryPtr[9] << 16 |
                    tlvEntryPtr[10] << 8 | tlvEntryPtr[11];
        rxPp0Counter[packetToSend] =
                    tlvEntryPtr[12] << 24 | tlvEntryPtr[13] << 16 |
                    tlvEntryPtr[14] << 8 | tlvEntryPtr[15];

        /* Restore original MAC addresses */
        prvTgfOamPacketHeaderDataReset();
    }

    /* AUTODOC: Compute near-end Loss Measurement result */
    lmResult = (txPp1Counter[1] - txPp1Counter[0]) -
                    (rxPp0Counter[1] - rxPp0Counter[0]);

    UTF_VERIFY_EQUAL2_STRING_MAC(
                    0, lmResult,
                    "Near-end Loss Measurement packet loss: %d, port %d",
                    lmResult,
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);

    /* AUTODOC: Compute far-end Loss Measurement result */
    lmResult = (txPp0Counter[1] - txPp0Counter[0]) -
                    (rxPp1Counter[1] - rxPp1Counter[0]);

    UTF_VERIFY_EQUAL2_STRING_MAC(
                    0, lmResult,
                    "Far-end Loss Measurement packet loss: %d, port %d",
                    lmResult,
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);

    PRV_UTF_LOG0_MAC("======= Single-ended LM protocol: unexcpected DMR counters =======\n");
    /* AUTODOC: Single-ended LM protocol: unexcpected DMR counters */
    for(packetToSend = 0; packetToSend < 2; packetToSend++)
    {
        /* AUTODOC: Phase 2: Send LMM packet from "CPU0" to "CPU1" */

        /* Set MEL = 5 */
        pduHeaderData.megLevel = 5;
        /* Set TLV offset */
        pduHeaderData.tlvOffset = 16;

        /* Set LMM OAM Opcode - CPU0*/
        pduHeaderData.opcode = PRV_TGF_LM_OPCODE_BYTE_42_CNS;
        /* AUTODOC: Set source mac address byte */
        prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.opcode);
        prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

        /* Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

        /* Parse from CPU0 OAM PDU header - LMM */
        prvTgfOamRxPduHeaderParse(capturePacketsArray[0], &pduHeaderData);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    5, pduHeaderData.megLevel, "Form CPU0 MEL Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_LM_OPCODE_BYTE_42_CNS, pduHeaderData.opcode, "From CPU0 Opcode Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);

        /* AUTODOC: Phase 3: Send LMM packet from "CPU1" to "CPU0" */

        /* AUTODOC: Swap MAC addresses */
        prvTgfOamPacketHeaderMacSwap();

        /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
        prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);

        /* AUTODOC: Reset EPLR billing entries on "PP0" and "PP1" */
        cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));

        prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            prvTgfBillingCntr.packetSizeMode =
                CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
            prvTgfBillingCntr.lmCntrCaptureMode =
                PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E;
        }
        rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_EGRESS_E,
                    POLICER_MASK(PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS +
                    PRV_TGF_OAM_EPCL_LM_RULE_PP0_PORT_INDEX_STC),
                    &prvTgfBillingCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);

        rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                    PRV_TGF_POLICER_STAGE_EGRESS_E,
                    POLICER_MASK(PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS +
                    PRV_TGF_OAM_EPCL_LM_RULE_PP1_PORT_INDEX_STC),
                    &prvTgfBillingCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);

        /* AUTODOC: Set LMR OAM Opcode - "CPU1" */
        pduHeaderData.opcode = PRV_TGF_LM_OPCODE_BYTE_43_CNS;
        /* AUTODOC: Set source mac address byte */
        prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.opcode);
        prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

        /* Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

        /* Parse from CPU0 OAM PDU header - LMR */
        prvTgfOamRxPduHeaderParse(capturePacketsArray[0], &pduHeaderData);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    5, pduHeaderData.megLevel, "Form CPU1 MEL Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_LM_OPCODE_BYTE_43_CNS, pduHeaderData.opcode, "From CPU1 Opcode Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS]);

        tlvEntryPtr = &pduHeaderData.tlvDataArr[0];

        /* Get counters for the first LMM/LMR packet */
        txPp0Counter[packetToSend] =
                    tlvEntryPtr[0] << 24 | tlvEntryPtr[1] << 16 |
                    tlvEntryPtr[2] << 8 | tlvEntryPtr[3];
        rxPp1Counter[packetToSend] =
                    tlvEntryPtr[4] << 24 | tlvEntryPtr[5] << 16 |
                    tlvEntryPtr[6] << 8 | tlvEntryPtr[7];
        txPp1Counter[packetToSend] =
                    tlvEntryPtr[8] << 24 | tlvEntryPtr[9] << 16 |
                    tlvEntryPtr[10] << 8 | tlvEntryPtr[11];
        rxPp0Counter[packetToSend] =
                    tlvEntryPtr[12] << 24 | tlvEntryPtr[13] << 16 |
                    tlvEntryPtr[14] << 8 | tlvEntryPtr[15];

        /* Restore original MAC addresses */
        prvTgfOamPacketHeaderDataReset();
    }

    /* AUTODOC: Compute near-end Loss Measurement result */
    lmResult = (txPp1Counter[1] - txPp1Counter[0]) -
                    (rxPp0Counter[1] - rxPp0Counter[0]);

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
                    0, lmResult,
                    "Near-end Loss Measurement: no loss packets");

    /* AUTODOC: Compute far-end Loss Measurement result */
    lmResult = (txPp0Counter[1] - txPp0Counter[0]) -
                    (rxPp1Counter[1] - rxPp1Counter[0]);

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
                    0, lmResult,
                    "Far-end Loss Measurement: no loss packets");
}

/**
* @internal tgfOamLossMeasurementGreenCounterSnapshotTest function
* @endinternal
*
* @brief   Loss Measurement Green Counter Snapshot Verification
*/
GT_VOID tgfOamLossMeasurementGreenCounterSnapshotTest
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;         /* return code */
    GT_U32          oamIndex;           /* OAM entry index */
    GT_U32          opCodeIndex;        /* opcode index range: 0..15 */
    GT_U32          opCode;             /* opcode value */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */
    PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr; /* Billing Counters */
    PRV_TGF_POLICER_BILLING_ENTRY_STC prvTgfBillingCntr1;/* Billing Counters */

    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));
    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    cpssOsMemSet(&prvTgfBillingCntr1, 0, sizeof(prvTgfBillingCntr1));

    usePortGroupsBmp  = GT_TRUE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* AUTODOC: Set LMM/LMR opcodes */
    for(opCodeIndex = 0; opCodeIndex < 2; opCodeIndex++)
    {
        switch(opCodeIndex)
        {
            case 0:
                /* Set OAM LM0 Opcode */
                opCode = PRV_TGF_LM_OPCODE_BYTE_42_CNS;
                break;
            default:
                /* Set OAM LM1 Opcode */
                opCode = PRV_TGF_LM_OPCODE_BYTE_43_CNS;
                break;
        }

        /* AUTODOC: Set LMM/LMR opcodes */
        rc = prvTgfOamOpcodeSet(
                PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                PRV_TGF_OAM_OPCODE_TYPE_LM_SINGLE_ENDED_E,
                opCodeIndex, opCode);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamOpcodeSet");

        rc = prvTgfOamOpcodeSet(
               PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
               PRV_TGF_OAM_OPCODE_TYPE_LM_COUNTED_E,
               opCodeIndex, opCode);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamOpcodeSet");

    }

    /* AUTODOC: Set IOAM configuration */
    for(oamIndex = 0; oamIndex < PRV_TGF_IOAM_PCL_RULE_NUM_STC; oamIndex++)
    {
        oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS + oamIndex;
        /* Get OAM common configuration */
        prvTgfOamConfigurationGet(&oamConfig);

        /* Enable LM counting and capture */
        oamConfig.oamEntry.lmCountingMode = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
        oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;
        /* Enable Opcode parsing */
        oamConfig.oamEntry.opcodeParsingEnable = GT_TRUE;
        oamConfig.oamEntry.packetCommandProfile =
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC;

        /* Set common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);
    }

    /* AUTODOC: Set LMM Opcode/Profile Packet Command for IOAM packets */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_LM_OPCODE_BYTE_42_CNS,
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC,
                    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                    2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeProfilePacketCommandEntrySet");

    /* AUTODOC: Set LMR Opcode/Profile Packet Command for IOAM packets */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_LM_OPCODE_BYTE_43_CNS,
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC,
                    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                    2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeProfilePacketCommandEntrySet");
    /* AUTODOC: Set OAM CPU code base for LMM/LMR packets trapped to CPU */
    rc = prvTgfOamCpuCodeBaseSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    CPSS_NET_FIRST_USER_DEFINED_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamCpuCodeBaseSet");

    PRV_UTF_LOG0_MAC("======= Single-ended LM protocol =======\n");


    /* AUTODOC: Phase 1: Send LMM packet from "CPU0" to "CPU1" */

    /* Set MEL = 3 */
    pduHeaderData.megLevel = 3;
    /* Set LMM OAM Opcode - CPU0*/
    pduHeaderData.opcode = PRV_TGF_LM_OPCODE_BYTE_42_CNS;

    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                pduHeaderData.megLevel);
    prvTgfOamTransmitPacket(
                &pduHeaderData,
                PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                PRV_TGF_OAM_BURST_COUNT_CNS,
                PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                0, /* Number of sent packets */
                1, /* Packets trapped to CPU */
                0, /* Exceptions */
                0  /* Billing counters */);

    /* AUTODOC: Get PP0 billing counter entry */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum,
                PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS + PRV_TGF_OAM_IPCL_LM_RULE_PP0_PORT_INDEX_STC,
                GT_TRUE,
                &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPolicerBillingEntryGet: %d", prvTgfDevNum);

    /* AUTODOC: Check PP0 green counter snapshot valid - TRUE */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, prvTgfBillingCntr.greenCntrSnapshotValid,
                                 "prvTgfBillingCntr.greenCntrSnapshotValid: %d",
                                 prvTgfBillingCntr.greenCntrSnapshotValid);

    /* AUTODOC: Phase 2: Send LMM packet from "CPU1" to "CPU0" */

    /* AUTODOC: Swap MAC addresses */
    prvTgfOamPacketHeaderMacSwap();
    /* Set MEL = 3 */
    pduHeaderData.megLevel = 3;
    /* Set LMR OAM Opcode - CPU1*/
    pduHeaderData.opcode = PRV_TGF_LM_OPCODE_BYTE_43_CNS;

    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                pduHeaderData.megLevel);
    prvTgfOamTransmitPacket(
                &pduHeaderData,
                PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                PRV_TGF_OAM_BURST_COUNT_CNS,
                PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                0, /* Number of sent packets */
                1, /* Packets trapped to CPU */
                0, /* Exceptions */
                0  /* Billing counters */);

    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum,
                PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS + PRV_TGF_OAM_IPCL_LM_RULE_PP1_PORT_INDEX_STC,
                GT_TRUE,
                &prvTgfBillingCntr1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPolicerBillingEntryGet: %d", prvTgfDevNum);

    /* AUTODOC: Check PP1 green counter snapshot valid - TRUE */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, prvTgfBillingCntr1.greenCntrSnapshotValid,
                "prvTgfBillingCntr1.greenCntrSnapshotValid: %d",
                prvTgfBillingCntr1.greenCntrSnapshotValid);

    /* AUTODOC: Check PP0/PP1 green counter snapshot - NOT equal to zero */
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, prvTgfBillingCntr.greenCntrSnapshot.l[0],
                    "prvTgfBillingCntr.greenCntrSnapshot.l[0] is zero");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, prvTgfBillingCntr1.greenCntrSnapshot.l[0],
                    "prvTgfBillingCntr1.greenCntrSnapshot.l[0] iz zero");

    /* AUTODOC: Check PP0/PP1 green counter snapshots - PP0 counter = PP1 counter */
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBillingCntr.greenCntrSnapshot.l[0],
                    prvTgfBillingCntr1.greenCntrSnapshot.l[0],
                    "PP0 greenCntrSnapshot not equal PP1 greenCntrSnapshot: %d, %d",
                    prvTgfBillingCntr.greenCntrSnapshot.l[0],
                    prvTgfBillingCntr1.greenCntrSnapshot.l[0]);

    /* Restore portGroupsBmp mode */
    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
}

