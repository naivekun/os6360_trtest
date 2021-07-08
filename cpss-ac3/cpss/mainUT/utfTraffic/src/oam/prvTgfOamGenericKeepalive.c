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
* @file prvTgfOamGenericKeepalive.c
*
* @brief OAM Generic Keepalive features testing
*
* @version   13
********************************************************************************
*/
#include <oam/prvTgfOamGenericKeepalive.h>

/* OAM keepalive aging period index */
#define PRV_TGF_OAM_KEEPALIVE_AGING_PERIOD_INDEX_STC            1

/* OAM keepalive aging threshold (periods) */
#define PRV_TGF_OAM_KEEPALIVE_AGING_THRESHOLD_STC               3

/* OAM keepalive excess period threshold */
#define PRV_TGF_OAM_EXCESS_KEEPALIVE_PERIOD_THRESHOLD_STC       8

/* OAM keepalive excess message threshold - minimal CCM during observation interval */
#define PRV_TGF_OAM_EXCESS_KEEPALIVE_MESSAGE_THRESHOLD_STC      2

/* OAM keepalive aging period value = 1 second */
#define PRV_TGF_OAM_KEEPALIVE_AGING_PERIOD_STC                  25000000

/* Keepalive Opcode Byte == 48 */
#define PRV_TGF_KEEPALIVE_OPCODE_BYTE_48_CNS                    0x30
/* Keepalive Opcode Byte == 49 */
#define PRV_TGF_KEEPALIVE_OPCODE_BYTE_49_CNS                    0x31
/* Keepalive Opcode Byte == 50 */
#define PRV_TGF_KEEPALIVE_OPCODE_BYTE_50_CNS                    0x32
/* Keepalive Opcode Byte == 51 */
#define PRV_TGF_KEEPALIVE_OPCODE_BYTE_51_CNS                    0x33

/* RDI status bit == 1 (default value is MSB of the UDB) */
#define PRV_TGF_RDI_STATUS_SET_CNS                              0x80
/* RDI status bit == 0 (default value is MSB of the UDB) */
#define PRV_TGF_RDI_STATUS_RESET_CNS                            0x0

/* Keeplive period field */
#define PRV_TGF_PERIOD_FIELD_VALUE_CNS                          0x5

/* Keeplive flow hash value */
#define PRV_TGF_FLOW_HASH_VALUE_CNS                             0x41F

/* Test port for Egress OAM hash client bit selection test */
#define PRV_TGF_EOAM_TEST_PORT                               prvTgfPortsArray[1]

/* PCL ID for Egress OAM hash client bit selection test */
#define PRV_TGF_EOAM_TEST_PCLID                              7

/* Burst packets number */
extern GT_U32 prvTgfOamBurstCount;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal tgfOamKeepAliveBasicSet function
* @endinternal
*
* @brief   Keepalive basic settings
*
* @param[out] oamConfigPtr             - pointer to OAM entry info
*                                       None
*/
static GT_VOID tgfOamKeepAliveBasicSet
(
    OUT PRV_TGF_OAM_COMMON_CONFIG_STC * oamConfigPtr
)
{
    GT_STATUS       rc = GT_OK;         /* return code */
    GT_U32          opCodeIndex;        /* opcode index range: 0..15 */
    GT_U32          opCode;             /* opcode value */
    GT_U64          keepalivePeriod;    /* keep alive aging period */

    /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Set Keepalive opcodes */
    for(opCodeIndex = 0; opCodeIndex < 4; opCodeIndex++)
    {
        switch(opCodeIndex)
        {
            case 0:
                /* Set Keepalive Opcode 0 */
                opCode = PRV_TGF_KEEPALIVE_OPCODE_BYTE_48_CNS;
                break;
            case 1:
                /* Set Keepalive Opcode 1 */
                opCode = PRV_TGF_KEEPALIVE_OPCODE_BYTE_49_CNS;
                break;
            case 2:
                /* Set Keepalive Opcode 2 */
                opCode = PRV_TGF_KEEPALIVE_OPCODE_BYTE_50_CNS;
                break;
            default:
                /* Set Keepalive Opcode 3 */
                opCode = PRV_TGF_KEEPALIVE_OPCODE_BYTE_51_CNS;
                break;
        }

        rc = prvTgfOamOpcodeSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_OPCODE_TYPE_KEEPALIVE_E,
                    opCodeIndex, opCode);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeSet");
    }

    cpssOsMemSet(oamConfigPtr, 0, sizeof(PRV_TGF_OAM_COMMON_CONFIG_STC));
    /* AUTODOC: Set IOAM configuration */
    oamConfigPtr->oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS;
    /* AUTODOC: Get OAM common configuration */
    prvTgfOamConfigurationGet(oamConfigPtr);

    /* AUTODOC: Enable keepalive aging */
    oamConfigPtr->oamEntry.keepaliveAgingEnable = GT_TRUE;
    oamConfigPtr->oamEntry.agingPeriodIndex =
    PRV_TGF_OAM_KEEPALIVE_AGING_PERIOD_INDEX_STC;
    /* AUTODOC: Set aging threshold = 3 */
    oamConfigPtr->oamEntry.agingThreshold =
    PRV_TGF_OAM_KEEPALIVE_AGING_THRESHOLD_STC;
    oamConfigPtr->oamEntry.ageState = 0;
    oamConfigPtr->oamEntry.protectionLocUpdateEnable = GT_FALSE;
    oamConfigPtr->oamEntry.hashVerifyEnable = GT_FALSE;
    oamConfigPtr->oamEntry.flowHash = 0;
    oamConfigPtr->oamEntry.lockHashValueEnable = 0;
    oamConfigPtr->oamEntry.excessKeepaliveDetectionEnable = GT_FALSE;
    oamConfigPtr->oamEntry.excessKeepalivePeriodCounter = 0;
    oamConfigPtr->oamEntry.excessKeepalivePeriodThreshold = 0;
    oamConfigPtr->oamEntry.excessKeepaliveMessageCounter = 0;
    oamConfigPtr->oamEntry.excessKeepaliveMessageThreshold = 0;
    oamConfigPtr->oamEntry.rdiCheckEnable = GT_FALSE;
    oamConfigPtr->oamEntry.rdiStatus = 0;
    oamConfigPtr->oamEntry.periodCheckEnable = GT_FALSE;
    oamConfigPtr->oamEntry.keepaliveTxPeriod = 0;

    /* AUTODOC: Reset exception configuration */
    cpssOsMemSet(&oamConfigPtr->oamExceptionConfig, 0, sizeof(PRV_TGF_OAM_EXCEPTION_CONFIG_STC));
    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(oamConfigPtr);

    keepalivePeriod.l[0] = PRV_TGF_OAM_KEEPALIVE_AGING_PERIOD_STC;
    keepalivePeriod.l[1] = 0;

    /* Set aging period value */
    prvTgfOamAgingPeriodEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_KEEPALIVE_AGING_PERIOD_INDEX_STC,
                    keepalivePeriod);

    /* AUTODOC: Set IOAM aging daemon disable */
    prvTgfOamAgingDaemonEnableSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    GT_FALSE);
}

/**
* @internal tgfOamKeepAliveBasicReset function
* @endinternal
*
* @brief   Keepalive basic reset
*/
static GT_VOID tgfOamKeepAliveBasicReset
(
    GT_VOID
)
{
    GT_U64          keepalivePeriod;            /* keep alive aging period */
    GT_U32          opCodeIndex;                /* opcode index range: 0..15 */
    GT_STATUS       rc = GT_OK;                 /* return code */
    PRV_TGF_OAM_EXCEPTION_TYPE_ENT  oamExceptionType;
    PRV_TGF_OAM_EXCEPTION_CONFIG_STC  oamExceptionConfig;

    /* AUTODOC: Set IOAM aging daemon disable */
    prvTgfOamAgingDaemonEnableSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    GT_FALSE);

    cpssOsMemSet(&oamExceptionConfig, 0, sizeof(oamExceptionConfig));

    /* AUTODOC: Set Keepalive opcodes */
    for(opCodeIndex = 0; opCodeIndex < 4; opCodeIndex++)
    {
        rc = prvTgfOamOpcodeSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_OPCODE_TYPE_KEEPALIVE_E,
                    opCodeIndex, 1);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeSet");
    }

    /* Restore portGroupsBmp mode */
    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    for(oamExceptionType = PRV_TGF_OAM_EXCEPTION_TYPE_MEG_LEVEL_E;
         oamExceptionType < PRV_TGF_OAM_EXCEPTION_TYPE_SUMMARY_E;
         oamExceptionType++)
    {
        rc = prvTgfOamExceptionConfigSet(
                        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                        oamExceptionType,
                        &oamExceptionConfig);
        UTF_VERIFY_EQUAL1_PARAM_MAC(
                        GT_OK, rc, "prvTgfOamExceptionConfigSet");
    }

    keepalivePeriod.l[0] = 0;
    keepalivePeriod.l[1] = 0;

    /* Reset aging period value */
    prvTgfOamAgingPeriodEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    PRV_TGF_OAM_KEEPALIVE_AGING_PERIOD_INDEX_STC,
                    keepalivePeriod);

}

/**
* @internal tgfOamKeepAliveAgeTest function
* @endinternal
*
* @brief   Keepalive age state verification
*/
GT_VOID tgfOamKeepAliveAgeTest
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;         /* return code */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */
    GT_U32          secondsDelay;       /* delay in seconds */

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    tgfOamKeepAliveBasicSet(&oamConfig);

    /* AUTODOC: Set age state = 3 */
    oamConfig.oamEntry.ageState = 3;
    oamConfig.oamExceptionType =
        PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_AGING_E;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set MEG level = 3 */
    pduHeaderData.megLevel = 3;
    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);

    /* AUTODOC: Set opcode = 48 */
    pduHeaderData.opcode = PRV_TGF_KEEPALIVE_OPCODE_BYTE_48_CNS;
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check age state = 0 */
    prvTgfOamConfigurationGet(&oamConfig);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    0, oamConfig.oamEntry.ageState, "OAM Entry Aged: index %d, port %d",
                    oamConfig.oamEntryIndex,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    1, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0 /* Billing counters */);

    /* AUTODOC: Set IOAM aging daemon enable */
    prvTgfOamAgingDaemonEnableSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    GT_TRUE);

    /* AUTODOC: Set age state = 0 */
    oamConfig.oamEntry.ageState = 0;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);


    /* AUTODOC: Send 1 broadcast packet to port 1 */
    rc = prvTgfStartTransmitingEth(
                    prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                    prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    secondsDelay = 5;
    /* Wait 5 seconds */
    cpssOsTimerWkAfter(secondsDelay * 1000);

    /* AUTODOC: Check age state = PRV_TGF_OAM_KEEPALIVE_AGING_THRESHOLD_STC */
    prvTgfOamConfigurationGet(&oamConfig);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_OAM_KEEPALIVE_AGING_THRESHOLD_STC,
                    oamConfig.oamEntry.ageState, "OAM Entry Aged: index %d, port %d",
                    oamConfig.oamEntryIndex,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_AGING_E,
                    1, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    1, /* Exceptions */
                    0 /* Billing counters */);

    tgfOamKeepAliveBasicReset();
}

/**
* @internal tgfOamKeepAliveExcessDetectionTest function
* @endinternal
*
* @brief   Keepalive excess detection verification
*/
GT_VOID tgfOamKeepAliveExcessDetectionTest
(
    GT_VOID
)
{
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */
    GT_U32 numOfTransmitPackets;
    GT_U32 keepaliveExcessPeriodThreshold =
        PRV_TGF_OAM_EXCESS_KEEPALIVE_PERIOD_THRESHOLD_STC;
    GT_U32 keepaliveExcessMessageThreshold =
        PRV_TGF_OAM_EXCESS_KEEPALIVE_MESSAGE_THRESHOLD_STC;

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

    tgfOamKeepAliveBasicSet(&oamConfig);

    /* AUTODOC: Set IOAM configuration */
    oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS;

    /* Get OAM common configuration */
    prvTgfOamConfigurationGet(&oamConfig);

    /* AUTODOC: Set excess keepalive aging enable */
    oamConfig.oamEntry.keepaliveAgingEnable = GT_TRUE;
    /* AUTODOC: Enable excess keepalive detection */
    oamConfig.oamEntry.excessKeepaliveDetectionEnable = GT_TRUE;
    /* AUTODOC: Set excess keepalive period threshold */
    oamConfig.oamEntry.excessKeepalivePeriodThreshold =
                    keepaliveExcessPeriodThreshold;
    /* AUTODOC: Set excess keepalive message threshold */
    oamConfig.oamEntry.excessKeepaliveMessageThreshold =
                    keepaliveExcessMessageThreshold;
    oamConfig.oamExceptionType =
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set MEG level = 3 */
    pduHeaderData.megLevel = 3;
    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                        PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                        pduHeaderData.megLevel);

    /* AUTODOC: Set opcode = 49 */
    pduHeaderData.opcode = PRV_TGF_KEEPALIVE_OPCODE_BYTE_49_CNS;

    numOfTransmitPackets = 3;
    /* AUTODOC: Phase 1: Send broadcast packets to port 1 */
    prvTgfOamTransmitPacket(
                        &pduHeaderData,
                        PRV_TGF_SEND_PORT_IDX_CNS,
                        numOfTransmitPackets,
                        PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check OAM exception keepalive message counter = PRV_TGF_OAM_EXCESS_KEEPALIVE_MESSAGE_THRESHOLD_STC */
    prvTgfOamConfigurationGet(&oamConfig);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                        keepaliveExcessMessageThreshold,
                        oamConfig.oamEntry.excessKeepaliveMessageCounter, "OAM Excess Message Counter: index %d, port %d",
                        oamConfig.oamEntryIndex,
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                        PRV_TGF_SEND_PORT_IDX_CNS,
                        PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_EXCESS_E,
                        numOfTransmitPackets, /* Number of sent packets */
                        0, /* Packets trapped to CPU */
                        1, /* Exceptions */
                        0 /* Billing counters */);

    /* AUTODOC: Set age threshold == 1 */
    oamConfig.oamEntry.agingThreshold = 1;
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set IOAM aging daemon enable */
    prvTgfOamAgingDaemonEnableSet(
                        PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                        GT_TRUE);

    cpssOsTimerWkAfter(500);

    /* AUTODOC: Check OAM exception keepalive message counter = 0 */
    prvTgfOamConfigurationGet(&oamConfig);
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(
                        0,
                        oamConfig.oamEntry.excessKeepaliveMessageCounter, "OAM Excess Message Counter: index %d, port %d",
                        oamConfig.oamEntryIndex,
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    keepaliveExcessPeriodThreshold = 4;
    keepaliveExcessMessageThreshold = 7;

    /* AUTODOC: Set excess keepalive period threshold */
    oamConfig.oamEntry.excessKeepalivePeriodThreshold =
                    keepaliveExcessPeriodThreshold;
    /* AUTODOC: Set excess keepalive message threshold */
    oamConfig.oamEntry.excessKeepaliveMessageThreshold =
                    keepaliveExcessMessageThreshold;

    /* AUTODOC: Reset excess keepalive message counters */
    oamConfig.oamEntry.excessKeepaliveMessageCounter = 0;
    oamConfig.oamEntry.excessKeepalivePeriodCounter = 0;

    prvTgfOamConfigurationSet(&oamConfig);

    cpssOsTimerWkAfter(2000);

    /* AUTODOC: Check OAM exception keepalive message counter = 4 */
    prvTgfOamConfigurationGet(&oamConfig);
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(
                        0,
                        oamConfig.oamEntry.excessKeepalivePeriodCounter, "OAM Excess Keepalive Period Counter: index %d, port %d",
                        oamConfig.oamEntryIndex,
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    tgfOamKeepAliveBasicReset();
}

/**
* @internal tgfOamKeepAliveRdiBitCheckingTest function
* @endinternal
*
* @brief   Keepalive RDI bit verification
*/
GT_VOID tgfOamKeepAliveRdiBitCheckingTest
(
    GT_VOID
)
{
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* AUTODOC: Set common OAM keepalive configurations */
    tgfOamKeepAliveBasicSet(&oamConfig);

    /* AUTODOC: Enable RDI bit checking */
    oamConfig.oamEntry.rdiCheckEnable = GT_TRUE;
    /* AUTODOC: Reset RDI status */
    oamConfig.oamEntry.rdiStatus = 0;
    oamConfig.oamExceptionType =
        PRV_TGF_OAM_EXCEPTION_TYPE_RDI_STATUS_CHANGED_E;
    oamConfig.oamExceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    oamConfig.oamExceptionConfig.cpuCode =
        CPSS_NET_FIRST_USER_DEFINED_E +
        PRV_TGF_OAM_EXCEPTION_TYPE_RDI_STATUS_CHANGED_E;
    oamConfig.oamExceptionConfig.summaryBitEnable = GT_TRUE;

    /* AUTODOC: Set specific OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set MEG level = 3 */
    pduHeaderData.megLevel = 3;
    /* AUTODOC: Set RDI status on */
    pduHeaderData.flags = PRV_TGF_RDI_STATUS_SET_CNS;
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
                        PRV_TGF_OAM_EXCEPTION_TYPE_RDI_STATUS_CHANGED_E,
                        0, /* Number of sent packets */
                        prvTgfOamBurstCount, /* Packets trapped to CPU */
                        prvTgfOamBurstCount, /* Exceptions */
                        0 /* Billing counters */);

    /* AUTODOC: Set common OAM keepalive configurations */
    tgfOamKeepAliveBasicSet(&oamConfig);

    /* AUTODOC: Enable RDI bit checking */
    oamConfig.oamEntry.rdiCheckEnable = GT_TRUE;

    /* AUTODOC: Set specific OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Reset RDI status */
    pduHeaderData.flags = 0;

    prvTgfOamTransmitPacket(
                        &pduHeaderData,
                        PRV_TGF_SEND_PORT_IDX_CNS,
                        PRV_TGF_OAM_BURST_COUNT_CNS,
                        PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                        PRV_TGF_SEND_PORT_IDX_CNS,
                        PRV_TGF_OAM_EXCEPTION_TYPE_RDI_STATUS_CHANGED_E,
                        1, /* Number of sent packets */
                        0, /* Packets trapped to CPU */
                        0, /* Exceptions */
                        0 /* Billing counters */);

    tgfOamKeepAliveBasicReset();
}

/**
* @internal tgfOamKeepAlivePeriodFieldCheckingTest function
* @endinternal
*
* @brief   Keepalive period field verification
*/
GT_VOID tgfOamKeepAlivePeriodFieldCheckingTest
(
    GT_VOID
)
{
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* AUTODOC: Set common OAM keepalive configurations */
    tgfOamKeepAliveBasicSet(&oamConfig);

    /* AUTODOC: Enable period field checking */
    oamConfig.oamEntry.periodCheckEnable = GT_TRUE;

    /* AUTODOC: Set keepalive period field */
    oamConfig.oamExceptionType =
                    PRV_TGF_OAM_EXCEPTION_TYPE_TX_PERIOD_E;
    oamConfig.oamExceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    oamConfig.oamExceptionConfig.cpuCode =
                    CPSS_NET_FIRST_USER_DEFINED_E +
                    PRV_TGF_OAM_EXCEPTION_TYPE_TX_PERIOD_E;
    oamConfig.oamExceptionConfig.summaryBitEnable = GT_TRUE;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set MEG level = 3 */
    pduHeaderData.megLevel = 3;
    /* AUTODOC: Set period status on */
    pduHeaderData.flags = PRV_TGF_PERIOD_FIELD_VALUE_CNS;

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
                    PRV_TGF_OAM_EXCEPTION_TYPE_TX_PERIOD_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

    /* AUTODOC: Set common OAM keepalive configurations */
    tgfOamKeepAliveBasicSet(&oamConfig);

    /* AUTODOC: Enable period field checking */
    oamConfig.oamEntry.periodCheckEnable = GT_TRUE;

    /* AUTODOC: Set specific OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Reset period status */
    pduHeaderData.flags = 0;
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Forwarded */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_TX_PERIOD_E,
                    1, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0 /* Billing counters */);

    tgfOamKeepAliveBasicReset();
}

/**
* @internal tgfOamKeepAliveFlowHashCheckingTest function
* @endinternal
*
* @brief   2.4.5  Keepalive Flow Hash Checking
*/
GT_VOID tgfOamKeepAliveFlowHashCheckingTest
(
    GT_VOID
)
{
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */
    GT_U32                          expectedTraps; /* expected number of traps */
    GT_U32                          numOfSendPackets; /* number of send packets */
    GT_U32                          noTrapWhenLockHashFalse; /* GT_FALSE - there is trap to CPU when lockHashValueEnable = GT_FALSE
                                                               GT_TRUE - there is no trap to CPU when lockHashValueEnable = GT_FALSE */

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* the device does not use Hash mismatch exception packet command when lockHashValueEnable = GT_FALSE starting from SIP 5.15 (BobK)
       Bobcat2 use Hash mismatch exception packet command regardless of lockHashValueEnable */
    noTrapWhenLockHashFalse = PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum);

    tgfOamKeepAliveBasicSet(&oamConfig);

    /* AUTODOC: Enable hash verify */
    oamConfig.oamEntry.hashVerifyEnable = GT_TRUE;
    /* AUTODOC: Set keepalive flow hash field */
    oamConfig.oamEntry.flowHash = PRV_TGF_FLOW_HASH_VALUE_CNS;
    /* AUTODOC: Enable lock hash value */
    oamConfig.oamEntry.lockHashValueEnable = GT_TRUE;
    oamConfig.oamExceptionType =
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E;
    oamConfig.oamExceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    oamConfig.oamExceptionConfig.cpuCode =
                    CPSS_NET_FIRST_USER_DEFINED_E +
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E;
    oamConfig.oamExceptionConfig.summaryBitEnable = GT_TRUE;

    /* Set specific OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

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

    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

    /* AUTODOC: Check OAM new flow hash value not locked */
    prvTgfOamConfigurationGet(&oamConfig);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_FLOW_HASH_VALUE_CNS,
                    oamConfig.oamEntry.flowHash, "OAM new flow hash locked: index %d, port %d",
                    oamConfig.oamEntryIndex,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Reset lock hash value */
    oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
    /* Set specific OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Trapped to CPU */
    expectedTraps = (noTrapWhenLockHashFalse) ? 0 : prvTgfOamBurstCount;
    numOfSendPackets = (noTrapWhenLockHashFalse) ? 1 : 0;
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
                    numOfSendPackets, /* Number of sent packets */
                    expectedTraps, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

    /* AUTODOC: Check OAM new flow hash value locked */
    prvTgfOamConfigurationGet(&oamConfig);

    /* AUTODOC: Check OAM flow lock hash enabled */
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    1,
                    oamConfig.oamEntry.lockHashValueEnable, "OAM lock hash value disabled: index %d, port %d",
                    oamConfig.oamEntryIndex,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(
                    PRV_TGF_FLOW_HASH_VALUE_CNS,
                    oamConfig.oamEntry.flowHash, "OAM flow hash not locked: index %d, port %d",
                    oamConfig.oamEntryIndex,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Send one more packet */
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Forward */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
                    1, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0 /* Billing counters */);

    /* AUTODOC: Set IOAM hash bit selection bits 0..5 */
    prvTgfOamHashBitSelectionSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                                 1, 4);

    /* AUTODOC: Reset lock hash value */
    oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
    /* Set specific OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Trapped to CPU */
    expectedTraps = (noTrapWhenLockHashFalse) ? 0 : prvTgfOamBurstCount;
    numOfSendPackets = (noTrapWhenLockHashFalse) ? 1 : 0;
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
                    numOfSendPackets, /* Number of sent packets */
                    expectedTraps, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0 /* Billing counters */);

    /* AUTODOC: Check OAM new flow hash value locked */
    prvTgfOamConfigurationGet(&oamConfig);

    /* AUTODOC: Check OAM flow lock hash enabled */
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    1,
                    oamConfig.oamEntry.lockHashValueEnable, "OAM lock hash value disabled: index %d, port %d",
                    oamConfig.oamEntryIndex,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(
                    PRV_TGF_FLOW_HASH_VALUE_CNS,
                    oamConfig.oamEntry.flowHash, "OAM flow hash not locked: index %d, port %d",
                    oamConfig.oamEntryIndex,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Send one more packet */
    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Forward */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_SEND_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
                    1, /* Number of sent packets */
                    0, /* Packets trapped to CPU */
                    0, /* Exceptions */
                    0 /* Billing counters */);

    tgfOamKeepAliveBasicReset();
}

/**
* @internal tgfIOamKeepAliveFlowHashVerificationBitSelectionTest function
* @endinternal
*
* @brief   Test Ingress OAM hash bit selection
*/
GT_VOID tgfIOamKeepAliveFlowHashVerificationBitSelectionTest
(
    GT_VOID
)
{
    PRV_TGF_OAM_COMMON_CONFIG_STC   oamConfig;    /* OAM configuration data */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */
    GT_U32                          expectedPackets; /* number of expected packets */
    GT_STATUS                       st;
                                                    /*DST    SRC    HASH  */
    GT_U8                           macs[] = {0x19, /*0x18 ^ 0x19 = 000001*/
                                              0x1B, /*0x18 ^ 0x1B = 000011*/
                                              0x1D, /*0x18 ^ 0x1D = 000101*/
                                              0x11, /*0x18 ^ 0x11 = 001001*/
                                              0x09, /*0x18 ^ 0x09 = 010001*/
                                              0x39  /*0x18 ^ 0x39 = 100001*/};
                                                     /*DST    SRC    HASH  */
    GT_U8                           expectedMacs[] = {0x01, /*000001*/
                                                      0x03, /*000011*/
                                                      0x05, /*000101*/
                                                      0x09, /*001001*/
                                                      0x11, /*010001*/
                                                      0x21  /*100001*/};
    GT_U32                          ii;
    GT_U32                          noTrapWhenLockHashFalse; /* GT_FALSE - there is trap to CPU when lockHashValueEnable = GT_FALSE
                                                                GT_TRUE - there is no trap to CPU when lockHashValueEnable = GT_FALSE */
    noTrapWhenLockHashFalse = PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum);
    expectedPackets = (noTrapWhenLockHashFalse) ? 1 : 0;

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* AUTODOC: Set MEG level = 3 */
    pduHeaderData.megLevel = 3;

    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.megLevel);

    tgfOamKeepAliveBasicSet(&oamConfig);


    /* AUTODOC: Enable hash verify */
    oamConfig.oamEntry.hashVerifyEnable = GT_TRUE;
    /* AUTODOC: Set keepalive flow hash field */
    oamConfig.oamEntry.flowHash = PRV_TGF_FLOW_HASH_VALUE_CNS;
    /* AUTODOC: Enable lock hash value */
    oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
    oamConfig.oamExceptionType =
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E;
    oamConfig.oamExceptionConfig.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    oamConfig.oamExceptionConfig.cpuCode =
                    CPSS_NET_FIRST_USER_DEFINED_E +
                    PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E;
    oamConfig.oamExceptionConfig.summaryBitEnable = GT_TRUE;

    /* Set specific OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    for(ii = 0; ii < sizeof(macs)/sizeof(macs[0]); ii++)
    {
        /* AUTODOC: Set the last byte of SA */
        prvTgfOamPacketHeaderSrcMacByteSet(5, macs[ii]);

        /* AUTODOC: Set hash selected bits */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, 0, ii + 1);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, 0,
            ii + 1);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(
            &pduHeaderData,
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS,
            PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
            expectedPackets, /* Number of sent packets */
            0, /* Packets trapped to CPU */
            prvTgfOamBurstCount, /* Exceptions */
            0 /* Billing counters */);

        /* AUTODOC: Reset FLOWID lock */
        prvTgfOamConfigurationGet(&oamConfig);
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(
            &pduHeaderData,
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS,
            PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
            expectedPackets, /* Number of sent packets */
            0, /* Packets trapped to CPU */
            prvTgfOamBurstCount, /* Exceptions */
            0  /* Billing counters */);

        /* AUTODOC: Check OAM new flow hash value not locked */
        prvTgfOamConfigurationGet(&oamConfig);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            expectedMacs[ii],
            oamConfig.oamEntry.flowHash,
            "OAM bad flow hash locked: index %d, port %d",
            oamConfig.oamEntryIndex,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: Reset FLOWID lock */
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);

        /* AUTODOC: Set hash selected bits to cut non-zero MSB of actual hash */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, ii, 1);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, 0,
            ii + 1);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(
            &pduHeaderData,
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS,
            PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
            expectedPackets, /* Number of sent packets */
            0, /* Packets trapped to CPU */
            prvTgfOamBurstCount, /* Exceptions */
            0 /* Billing counters */);

        /* AUTODOC: Reset FLOWID lock */
        prvTgfOamConfigurationGet(&oamConfig);
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(
            &pduHeaderData,
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS,
            PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
            expectedPackets, /* Number of sent packets */
            0, /* Packets trapped to CPU */
            prvTgfOamBurstCount, /* Exceptions */
            0  /* Billing counters */);

        /* AUTODOC: Check OAM new flow hash value not locked */
        prvTgfOamConfigurationGet(&oamConfig);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            1,
            oamConfig.oamEntry.flowHash,
            "OAM bad flow hash locked: index %d, port %d",
            oamConfig.oamEntryIndex,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: Reset FLOWID lock */
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);

        if(0 == ii)
        {
            continue;
        }
        /* AUTODOC: Set hash selected bits to one non-zero bit */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, 0, ii);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, 0,
            ii + 1);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(
            &pduHeaderData,
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS,
            PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
            expectedPackets, /* Number of sent packets */
            0, /* Packets trapped to CPU */
            prvTgfOamBurstCount, /* Exceptions */
            0 /* Billing counters */);

        /* AUTODOC: Reset FLOWID lock */
        prvTgfOamConfigurationGet(&oamConfig);
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(
            &pduHeaderData,
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS,
            PRV_TGF_OAM_NO_DELAY_CNS);

        /* AUTODOC: Check results - Trapped to CPU */
        prvTgfDefOamGeneralTrafficCheck(
            PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_EXCEPTION_TYPE_KEEPALIVE_INVALID_HASH_E,
            expectedPackets, /* Number of sent packets */
            0, /* Packets trapped to CPU */
            prvTgfOamBurstCount, /* Exceptions */
            0  /* Billing counters */);

        /* AUTODOC: Check OAM new flow hash value not locked */
        prvTgfOamConfigurationGet(&oamConfig);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            1,
            oamConfig.oamEntry.flowHash,
            "OAM bad flow hash locked: index %d, port %d",
            oamConfig.oamEntryIndex,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: Reset FLOWID lock */
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);
    }

    tgfOamKeepAliveBasicReset();
}

/**
* @internal tgfEOamKeepAliveFlowHashVerificationBitSelectionConfigSet function
* @endinternal
*
* @brief   Config test for Egress OAM hash bit selection
*/
GT_VOID tgfEOamKeepAliveFlowHashVerificationBitSelectionConfigSet
(
    GT_VOID
)
{
    GT_STATUS st;

    /* AUTODOC: Set common bridge configuration */
    prvTgfOamBridgeConfigurationSet();

    /* AUTODOC: enable egress Policy */
    prvTgfPclEgressPolicyEnable(GT_TRUE);

    st = prvTgfPclDefPortInitExt1(PRV_TGF_EOAM_TEST_PORT,
        CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_EOAM_TEST_PCLID,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, st,
        "prvTgfPclDefPortInitExt1 %d %d %d %d %d %d %d",
        PRV_TGF_EOAM_TEST_PORT,
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_EOAM_TEST_PCLID,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);

    st = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum,
        PRV_TGF_EOAM_TEST_PORT,
        PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,
        GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfPclEgressPclPacketTypesSet: %d %d %d %d",
        prvTgfDevNum,
        PRV_TGF_EOAM_TEST_PORT,
        PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,
        GT_TRUE);

}

/**
* @internal tgfEOamKeepAliveFlowHashVerificationBitSelectionOamEntryGet function
* @endinternal
*
* @brief   Get the OAM entry in specific port group
*/
static GT_STATUS tgfEOamKeepAliveFlowHashVerificationBitSelectionOamEntryGet
(
    IN  PRV_TGF_OAM_STAGE_TYPE_ENT          stage,
    IN  GT_U32                              entryIndex,
    OUT PRV_TGF_OAM_ENTRY_STC               *entryPtr
)
{
    GT_STATUS st;
    GT_BOOL                 saveUsePortGroupsBmp = usePortGroupsBmp;
    GT_PORT_GROUPS_BMP      saveCurrPortGroupsBmp = currPortGroupsBmp;
    GT_PHYSICAL_PORT_NUM    physicalPortNum;
    GT_U32                  portGroupId = 0;

    /* Because of the way the test is built, we expect the result in a particular port group */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* Use the ingress port */
        physicalPortNum = CPSS_CPU_PORT_NUM_CNS;
    }
    else
    {
        /* Use the egress port */
        physicalPortNum = PRV_TGF_EOAM_TEST_PORT;
    }
    st = prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert(prvTgfDevNum,
                                                            physicalPortNum,
                                                            &portGroupId);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert(%d) FAILED, rc = [%d]", physicalPortNum, st);

        return st;
    }

    /* Set use port group BMP */
    usePortGroupsBmp = GT_TRUE;
    currPortGroupsBmp = 1 << portGroupId;

    /* Call the function */
    st = prvTgfOamEntryGet(
        stage,
        entryIndex,
        entryPtr);
    /* Return code checking is expected at the caller */

    /* Restore use port group BMP */
    usePortGroupsBmp = saveUsePortGroupsBmp;
    currPortGroupsBmp = saveCurrPortGroupsBmp;

    return st;
}

/**
* @internal tgfEOamKeepAliveFlowHashVerificationBitSelectionTest function
* @endinternal
*
* @brief   Test Egress OAM hash bit selection
*/
GT_VOID tgfEOamKeepAliveFlowHashVerificationBitSelectionTest
(
    GT_VOID
)
{
    PRV_TGF_OAM_PDU_HEADER_DATA_STC  pduHeaderData;  /* PDU header data */
    GT_STATUS                        st;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;           /* TCAM rule mask */
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;        /* TCAM rule data */
    PRV_TGF_PCL_ACTION_STC           action;         /* PCL action */
    GT_U32                           ruleIdx;
    PRV_TGF_OAM_COMMON_CONFIG_STC    oamConfig;      /* OAM configuration */
    PRV_TGF_OAM_ENTRY_STC            oamEntry;
    GT_U8                            macs[] = {0x19, /*0x18 ^ 0x19 = 000001*/
                                             0x1B, /*0x18 ^ 0x1B = 000011*/
                                             0x1D, /*0x18 ^ 0x1D = 000101*/
                                             0x11, /*0x18 ^ 0x11 = 001001*/
                                             0x09, /*0x18 ^ 0x09 = 010001*/
                                             0x39  /*0x18 ^ 0x39 = 100001*/};
                                                      /*DST    SRC    HASH  */
    GT_U8                            expectedMacs[] = {0x01, /*000001*/
                                                       0x03, /*000011*/
                                                       0x05, /*000101*/
                                                       0x09, /*001001*/
                                                       0x11, /*010001*/
                                                       0x21  /*100001*/};
    GT_U32                           ii;

    /* AUTODOC: clear mask, pattern and action */
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&pattern,0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    ruleIdx = appDemoDxChTcamEpclBaseIndexGet(prvTgfDevNum) + 1;

    /* Ethertype */
    mask.ruleEgrStdNotIp.etherType = 0xFFFF;
    pattern.ruleEgrStdNotIp.etherType = TGF_ETHERTYPE_8902_CFM_TAG_CNS;

    /* Set action */
    action.egressPolicy = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable = GT_TRUE;
    action.oam.oamProfile = PRV_TGF_OAM_PROFILE_INDEX_CNS;
    action.flowId = PRV_TGF_EPCL_ACTION_FLOW_ID_CNS; /* EPCL Action flow ID */

    /* Set PCL rule */
    st = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E, ruleIdx,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvTgfPclRuleSet: %d %d",
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E, ruleIdx);

    /* Set OAM configuration  */
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

    oamConfig.oamEnable = GT_TRUE;
    oamConfig.oamEntryIndex = PRV_EOAM_DM_ENTRY_START_INDEX_CNS;
    oamConfig.oamEntry.hashVerifyEnable = GT_TRUE;
    oamConfig.oamEntry.keepaliveAgingEnable = GT_TRUE;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* AUTODOC: Set MEG level = 3 */
    pduHeaderData.megLevel = 3;

    for(ii = 0; ii < sizeof(macs)/sizeof(macs[0]); ii++)
    {
        /* AUTODOC: Set the last byte of SA */
        prvTgfOamPacketHeaderSrcMacByteSet(5, macs[ii]);

        /* AUTODOC: Set hash selected bits */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 0, ii + 1);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 0,
            ii + 1);

        /* Set flow hash */
        oamConfig.oamEntry.flowHash = expectedMacs[ii];
        prvTgfOamConfigurationSet(&oamConfig);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(&pduHeaderData, PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS, PRV_TGF_OAM_NO_DELAY_CNS);

        /* Get OAM entry */
        st = tgfEOamKeepAliveFlowHashVerificationBitSelectionOamEntryGet(
            PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
            oamConfig.oamEntryIndex,
            &oamEntry);

        UTF_VERIFY_EQUAL4_STRING_MAC(
            expectedMacs[ii],
            oamEntry.flowHash,
            "OAM bad flow hash locked: index %d, port %d. %04X instead %04X.",
            oamConfig.oamEntryIndex,
            PRV_TGF_EOAM_TEST_PORT,
            expectedMacs[ii], oamEntry.flowHash);

        /* AUTODOC: Reset FLOWID lock */
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);

        /* AUTODOC: Set single bit for hash */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, ii, 1);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 0,
            ii + 1);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(&pduHeaderData, PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS, PRV_TGF_OAM_NO_DELAY_CNS);

        /* Get OAM entry */
        st = tgfEOamKeepAliveFlowHashVerificationBitSelectionOamEntryGet(
            PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
            oamConfig.oamEntryIndex,
            &oamEntry);

        UTF_VERIFY_EQUAL4_STRING_MAC(
            1,
            oamEntry.flowHash,
            "OAM bad flow hash locked: index %d, port %d. %04X instead %04X.",
            oamConfig.oamEntryIndex,
            PRV_TGF_EOAM_TEST_PORT,
            1, oamEntry.flowHash);

        /* AUTODOC: Reset FLOWID lock */
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;

        if(0 == ii)
        {
            continue;
        }

        /* AUTODOC: Set hash selected bits to cut non-zero MSB of actual hash */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 0, ii);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 0,
            ii + 1);

        /* AUTODOC: Transmit packet */
        prvTgfOamTransmitPacket(&pduHeaderData, PRV_TGF_SEND_PORT_IDX_CNS,
            PRV_TGF_OAM_BURST_COUNT_CNS, PRV_TGF_OAM_NO_DELAY_CNS);

        /* Get OAM entry */
        st = tgfEOamKeepAliveFlowHashVerificationBitSelectionOamEntryGet(
            PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
            oamConfig.oamEntryIndex,
            &oamEntry);

        UTF_VERIFY_EQUAL4_STRING_MAC(
            1,
            oamEntry.flowHash,
            "OAM bad flow hash locked: index %d, port %d. %04X instead %04X.",
            oamConfig.oamEntryIndex,
            PRV_TGF_EOAM_TEST_PORT,
            1, oamEntry.flowHash);

        /* AUTODOC: Reset FLOWID lock */
        oamConfig.oamEntry.lockHashValueEnable = GT_FALSE;
        prvTgfOamConfigurationSet(&oamConfig);
    }
}

/**
* @internal tgfEOamKeepAliveFlowHashVerificationBitSelectionRestore function
* @endinternal
*
* @brief   Restore configuration of test for Egress OAM hash bit selection
*/
GT_VOID tgfEOamKeepAliveFlowHashVerificationBitSelectionRestore
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    GT_U32                      ruleIdx;
    GT_STATUS                   st;

    /* AUTODOC: Disable egress Policy */
    prvTgfPclEgressPolicyEnable(GT_FALSE);

    /* clear PCL configuration table */
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum   = prvTgfDevNum;
    interfaceInfo.devPort.portNum    = PRV_TGF_EOAM_TEST_PORT;

    lookupCfg.pclId                  = PRV_TGF_EOAM_TEST_PCLID;
    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    st = prvTgfPclCfgTblSet(
                &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvTgfPclCfgTblSet %d %d",
        CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E);

    ruleIdx = appDemoDxChTcamEpclBaseIndexGet(prvTgfDevNum) + 1;

    /* invalidate PCL rules */
    st = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E, ruleIdx,
        GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
        "prvTgfPclRuleValidStatusSet: %d %d %d",
        CPSS_PCL_RULE_SIZE_EXT_E, ruleIdx, GT_FALSE);

    /* AUTODOC: Clear EPCL Engine configuration tables */
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E);

    st = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum,
        PRV_TGF_EOAM_TEST_PORT,
        PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,
        GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfPclEgressPclPacketTypesSet: %d %d %d %d",
        prvTgfDevNum,
        PRV_TGF_EOAM_TEST_PORT,
        PRV_TGF_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E,
        GT_FALSE);

    /* Reset hash bit selection to register default value */
    st = prvTgfOamHashBitSelectionSet(
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, 0, 11);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
        "prvTgfOamHashBitSelectionSet %d %d %d",
        PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, 0, 11);

    /* Restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();

    /* AUTODOC: Restore common bridge configuration */
    prvTgfOamBridgeConfigurationRestore();

    /* AUTODOC: Restore original MAC addresses */
    prvTgfOamPacketHeaderDataReset();

}

