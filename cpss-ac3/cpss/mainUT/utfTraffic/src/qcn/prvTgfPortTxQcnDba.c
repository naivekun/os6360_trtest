/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file prvTgfPortTxQcnDba.c
*
* @brief "QCN DBA" enhanced UT for Qcn Trigger APIs
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
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <common/tgfQcnGen.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>
#include <extUtils/iterators/extUtilDevPort.h>

#define PRV_TGF_RECV_PORT_IDX_CNS 1
#define PRV_TGF_SEND_PORT_IDX_CNS 0
#define PRV_TGF_CAPTURE_PORT_IDX_CNS 19
#define PRV_TGF_QCN_CPU_PORT_CNS 81
#define CPSS_TC_RANGE_CNS 8
#define CPSS_SIP6_TC_RANGE_CNS 16
#define PRV_TGF_TD_PROFILE_CNS CPSS_PORT_TX_DROP_PROFILE_7_E

#define PRV_TGF_QCN_COUNT_OF_FRAMES_TO_TRANSMIT_CNS 4
#define PRV_TGF_QCN_COUNT_OF_FRAMES_DBA_QCN_TO_TRANSMIT_CNS 50
#define PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS /*8 - 0.125 and 8 are not supported yet*/ 6
#define PRV_TGF_COUNT_OF_SIP6_QUEUE_TRANSMITTING_CNS 12
#define PRV_TGF_VLANID_CNS 7
#define PRV_TGF_TRAFFIC_CLASS_CNS 7
#define PRV_TGF_SIP6_TRAFFIC_CLASS_CNS 15
#define PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS 50
#define PRV_TGF_RCVPRT_COS_PROFILE_CNS 1000
#define PRV_TGF_CN_SAMPLE_INTERVAL_CNS 0x4000
#define PRV_TGF_CN_SAMPLE_INTERVAL_GRANULARITY_CNS 16
#define PRV_TGF_SAMPLE_TABLE_ENTRY_CNS 8

/* Rx buffer size */
#define TGF_RX_BUFFER_MAX_SIZE_CNS   0x3FFF

/* Macros for QCN Frame */
#define TGF_QCN_DA_CNS 6
#define TGF_QCN_SA_CNS 6
#define TGF_QCN_VLAN_CNS 4
#define TGF_QCN_ETHER_TYPE_CNS 2
#define TGF_QCN_HDR_CNS 24
#define TGF_QCN_HDR_SDU_LENGTH_CNS 2
#define TGF_QCN_VLAN_TPID_LENGTH_CNS 2

extern CPSS_OS_MEM_CMP_FUNC           cpssOsMemCmp;

/* Alpha for dynamic buffers allocation per {TC,DP} */
static CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
                    prvTgfQcnDbaQueueAlpha[PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS] = {
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E
};

/* Alpha for dynamic buffers allocation per {TC,DP} or port profile */
static CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
                    prvTgfSip6QcnDbaQueueAlpha[PRV_TGF_COUNT_OF_SIP6_QUEUE_TRANSMITTING_CNS] = {
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E
};

/* guaranteed buffers limit per TC */
static GT_U32 prvTgfQcnDbaQueueLimits[PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS] = {
    10,    /* for Alpha ratio 0.0 */
    10,    /* for Alpha ratio 0.25 */
    10,    /* for Alpha ratio 0.5 */
    10,    /* for Alpha ratio 1.0 */
    10,    /* for Alpha ratio 2.0 */
    1      /* for Alpha ratio 4.0 */
};

/* guaranteed buffers limit per TC or port */
static GT_U32 prvTgfSip6QcnDbaQueueLimits[PRV_TGF_COUNT_OF_SIP6_QUEUE_TRANSMITTING_CNS] = {
    10,    /* for Alpha ratio 0.0 */
    10,    /* for Alpha ratio 0.25 */
    10,    /* for Alpha ratio 0.5 */
    10,    /* for Alpha ratio 1.0 */
    10,    /* for Alpha ratio 2.0 */
    10,    /* for Alpha ratio 4.0 */
    10,    /* for Alpha ratio 0.125 */
    10,    /* for Alpha ratio 8.0 */
    10,    /* for Alpha ratio 0.03125 */
    10,    /* for Alpha ratio 0.0625 */
    10,    /* for Alpha ratio 16.0 */
    1,     /* for Alpha ratio 32.0 */
};

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

/* Convert Tail Drop DBA Alpha to ratio value (GT_FLOAT32) */
#define PRV_TGF_PORT_TX_QCN_DBA_ALPHA_TO_RATIO_CONVERT_MAC(alpha, ratio) \
    do {                                                                 \
        switch (alpha)                                                   \
        {                                                                \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                   \
                ratio = (0.0);                                           \
                break;                                                   \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                \
                ratio = (0.25);                                          \
                break;                                                   \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                 \
                ratio = (0.5);                                           \
                break;                                                   \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                   \
                ratio = (1.0);                                           \
                break;                                                   \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                   \
                ratio = (2.0);                                           \
                break;                                                   \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                   \
                ratio = (4.0);                                           \
                break;                                                   \
            default:                                                     \
                return GT_BAD_PARAM;                                     \
        }                                                                \
    } while (0)

/* Convert Tail Drop DBA Alpha to ratio value for Falcon (GT_FLOAT32) */
#define PRV_TGF_PORT_TX_SIP6_QCN_DBA_ALPHA_TO_RATIO_CONVERT_MAC(alpha, ratio) \
    do {                                                                      \
        switch (alpha)                                                        \
        {                                                                     \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                        \
                ratio = (0.0);                                                \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E:                  \
                ratio = (0.03125);                                            \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E:                   \
                ratio = (0.0625);                                             \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E:                    \
                ratio = (0.125);                                              \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                     \
                ratio = (0.25);                                               \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                      \
                ratio = (0.5);                                                \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                        \
                ratio = (1.0);                                                \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                        \
                ratio = (2.0);                                                \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                        \
                ratio = (4.0);                                                \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E:                        \
                ratio = (8.0);                                                \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E:                       \
                ratio = (16.0);                                               \
                break;                                                        \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E:                       \
                ratio = (32.0);                                               \
                break;                                                        \
            default:                                                          \
                return GT_BAD_PARAM;                                          \
        }                                                                     \
    } while (0)

/**
* @internal prvTgfPortTxQcnDbaQueueExpectedAmountOfBuffers function
* @endinternal
*
* @brief   Calculate expected amount of available buffers (include dynamic
*         allocation).
* @param[in] numOfTransmit            - number of transmitting
*
* @param[out] expectedValue           - expected amount of available buffers
*                                       None.
*/
static GT_STATUS prvTgfPortTxQcnDbaQueueExpectedAmountOfBuffers
(
    IN  GT_U32  numOfTransmit,
    OUT GT_U32  *expectedValue
)
{
    GT_U32      dynamicFactor, usedBuffers;
    GT_FLOAT32  ratio;

    /* Convert enum Alpha to float */
    PRV_TGF_PORT_TX_QCN_DBA_ALPHA_TO_RATIO_CONVERT_MAC(
            prvTgfQcnDbaQueueAlpha[numOfTransmit], ratio);

    /* test uses guarantied number of buffers before DBA start to work */
    usedBuffers = prvTgfQcnDbaQueueLimits[5];

    /*
       Dynamic factor calculated as:

                                                   Alpha
          DynamicFactor = FreeBuffers * ----------------------------
                                         1 + sumOfAllDestPortsRatio

       where sumOfAllDestPortsRatio - sum of Alphas of all destination ports.
       On this test we set Alphas for Drop Profile 0 (default profile)
       to 0 - so we have Alpha0 on all ports except receive. On just one
       port (receive) we use Drop Profile 7 with different Alphas (see
       variable prvTgfTailDropDbaPortDbaPortAlpha). So:

          sumOfAllDestPortsRatio = ratio    - because just one port (receive)
                                              will have non-zero Alpha
    */
    dynamicFactor = (GT_U32)((PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS - usedBuffers) /* Free Buffers */
                    * (ratio / (1 + ratio)));              /* Alpha / (1 + sumOfAllDestPortsRatio) */

    /* Limit = Guaranteed Buffers + Dynamic Factor */
    *expectedValue = PRV_TGF_QCN_COUNT_OF_FRAMES_DBA_QCN_TO_TRANSMIT_CNS - (usedBuffers + dynamicFactor);

    return GT_OK;
}

/**
* @internal prvTgfPortTxSip6QcnDbaQueueExpectedAmountOfBuffers function
* @endinternal
*
* @brief   Calculate expected amount of available buffers (include dynamic
*         allocation).
* @param[in] numOfTransmit            - number of transmitting
*
* @param[out] expectedValue           - expected amount of available buffers
*                                       None.
*/
static GT_STATUS prvTgfPortTxSip6QcnDbaQueueExpectedAmountOfBuffers
(
    IN  GT_U32  numOfTransmit,
    OUT GT_U32  *expectedValue
)
{
    GT_U32      dynamicFactor, usedBuffers;
    GT_FLOAT32  ratio;

    /* Convert enum Alpha to float */
    PRV_TGF_PORT_TX_SIP6_QCN_DBA_ALPHA_TO_RATIO_CONVERT_MAC(
            prvTgfSip6QcnDbaQueueAlpha[numOfTransmit], ratio);

    /* test uses guarantied number of buffers before DBA start to work */
    usedBuffers = prvTgfSip6QcnDbaQueueLimits[11];

    /*
       Dynamic factor calculated as:

                                                   Alpha
          DynamicFactor = FreeBuffers * ----------------------------
                                         1 + sumOfAllDestPortsRatio

       where sumOfAllDestPortsRatio - sum of Alphas of all destination ports.
       On this test we set Alphas for Drop Profile 0 (default profile)
       to 0 - so we have Alpha0 on all ports except receive. On just one
       port (receive) we use Drop Profile 7 with different Alphas (see
       variable prvTgfTailDropDbaPortDbaPortAlpha). So:

          sumOfAllDestPortsRatio = ratio    - because just one port (receive)
                                              will have non-zero Alpha
    */
    dynamicFactor = (GT_U32)((PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS - usedBuffers) /* Free Buffers */
                    * (ratio / (1 + ratio)));              /* Alpha / (1 + sumOfAllDestPortsRatio) */

    /* Limit = Guaranteed Buffers + Dynamic Factor */
    *expectedValue = PRV_TGF_QCN_COUNT_OF_FRAMES_DBA_QCN_TO_TRANSMIT_CNS - (usedBuffers + dynamicFactor);

    return GT_OK;
}

/**
* @internal tgfPortTxQcnQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxQcnQueueResources test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxQcnQueueResourcesConfigure
(
    GT_BOOL configSet,
    GT_BOOL ingressPktHdr
)
{
    GT_STATUS                                     rc = GT_OK;
    GT_U8                                         tc;
    GT_U8                                         entryIndex;
    static CPSS_QOS_ENTRY_STC                     qosRecvPortConfigRestore;
    CPSS_QOS_ENTRY_STC                            qosRecvPortConfig;
    static PRV_TGF_COS_PROFILE_STC                cosProfileRestore;
    PRV_TGF_COS_PROFILE_STC                       cosProfileConfig;
    static CPSS_QOS_PORT_TRUST_MODE_ENT           trustModeRestore;
    GT_U8                                         vlanTags[2];
    GT_U32                                        vlanPortsArray[2];
    static CPSS_DXCH_PORT_CN_MODE_ENT             cnModePtrRestore;
    static GT_U32                                 etherTypePtrRestore;
    static CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfgPtrRestore;
    static CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;
    static CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC cnSampleEntryPtrRestore[CPSS_TC_RANGE_CNS];
    static CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC cnSampleEntry;
    static GT_BOOL                                enablePtrRestore;
    static GT_HW_DEV_NUM                          targetHwDevPtrRestore;
    static GT_PORT_NUM                            targetPortPtrRestore;
    static GT_HW_DEV_NUM                          targetHwDev;
    static CPSS_PORT_TX_DROP_PROFILE_SET_ENT      profileSetPtrRestore;
    static CPSS_PORT_CNM_GENERATION_CONFIG_STC    cnmGenerationCfgPtrRestore;
    static CPSS_PORT_CNM_GENERATION_CONFIG_STC    cnmGenerationCfg;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        cnProfileCfgParams;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        defCnProfileCfgParams;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        profileParamsPtr;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        cnProfileCfgParamsRestore[CPSS_TC_RANGE_CNS];
    static GT_BOOL                                cnmTermStatusSendPortPtrRestore;
    static GT_BOOL                                cnmTermStatusCPUPortPtrRestore;
    static GT_U32                                 portForCpuRestore;

    if(configSet)
    {
        portForCpuRestore = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS] = PRV_TGF_QCN_CPU_PORT_CNS;

        /* AUTODOC: save cn enable/disable mechanism */
        rc = prvTgfPortCnModeEnableGet(prvTgfDevNum, &cnModePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnModeEnableGet: %d", cnModePtrRestore);

        /* AUTODOC: set cn enable mechanism */
        rc = prvTgfPortCnModeEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnModeEnableSet");

        /* AUTODOC: Save the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeGet(prvTgfDevNum, &etherTypePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeGet %d",
            etherTypePtrRestore);

        /* AUTODOC: Set the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeSet(prvTgfDevNum, 0x22E9);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeSet");

        /* AUTODOC: Save the Fb calculation configuration */
        rc = prvTgfPortCnFbCalcConfigGet(prvTgfDevNum, &fbCalcCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigGet %p");

        /* AUTODOC: Set the Fb calculation configuration */
        fbCalcCfg.wExp = 0;
        fbCalcCfg.fbLsb = 0;
        fbCalcCfg.deltaEnable = GT_TRUE;
        fbCalcCfg.fbMin = 0;
        fbCalcCfg.fbMax = 50;
        rc = prvTgfPortCnFbCalcConfigSet(prvTgfDevNum, &fbCalcCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigSet");

        /* AUTODOC: Save the CN sample interval entry */
        for (entryIndex = 0; entryIndex < PRV_TGF_SAMPLE_TABLE_ENTRY_CNS; entryIndex++)
        {
            rc = prvTgfPortCnSampleEntryGet(prvTgfDevNum, entryIndex, &cnSampleEntryPtrRestore[entryIndex]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnSampleEntrySet");
        }

        /* AUTODOC: Set the CN sample interval entry */
        cnSampleEntry.interval = PRV_TGF_CN_SAMPLE_INTERVAL_CNS / PRV_TGF_CN_SAMPLE_INTERVAL_GRANULARITY_CNS;
        cnSampleEntry.randBitmap = 0;
        for (entryIndex = 0; entryIndex < PRV_TGF_SAMPLE_TABLE_ENTRY_CNS; entryIndex++)
        {
            rc = prvTgfPortCnSampleEntrySet(prvTgfDevNum, entryIndex, &cnSampleEntry);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnSampleEntrySet");
        }

        /* AUTODOC: Save the CNM Target Assignment Mode */
        rc = prvTgfPortCnQueueStatusModeEnableGet(prvTgfDevNum, &enablePtrRestore,
            &targetHwDevPtrRestore, &targetPortPtrRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableGet %d %d %d",
            enablePtrRestore, targetHwDevPtrRestore, targetPortPtrRestore);

        /* AUTODOC: Set the CNM Target Assignment Mode */
        rc = extUtilHwDeviceNumberGet(prvTgfDevNum, &targetHwDev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "extUtilHwDeviceNumberGet");
        rc = prvTgfPortCnQueueStatusModeEnableSet(prvTgfDevNum, GT_TRUE,
            targetHwDev, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableSet");

        /* AUTODOC: Save the receive port (1) drop profile */
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], &profileSetPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet %d",
            profileSetPtrRestore);

        /* AUTODOC: Set the receive port (1) drop profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], PRV_TGF_TD_PROFILE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

        /* AUTODOC: Save the CNM Generation Config */
        rc = prvTgfPortCnMessageGenerationConfigGet(prvTgfDevNum, &cnmGenerationCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigGet");

        /* AUTODOC: Set the CNM Generation Config */
        cnmGenerationCfg.qosProfileId = 0;
        cnmGenerationCfg.isRouted = 0;
        cnmGenerationCfg.overrideUp = GT_TRUE;
        cnmGenerationCfg.cnmUp = 0;
        cnmGenerationCfg.defaultVlanId = 0;
        cnmGenerationCfg.scaleFactor = 256;
        cnmGenerationCfg.version = 0;
        cnmGenerationCfg.cpidMsb[0] = 0;
        cnmGenerationCfg.cpidMsb[1] = 0;
        cnmGenerationCfg.cpidMsb[2] = 0;
        cnmGenerationCfg.cpidMsb[3] = 0;
        cnmGenerationCfg.cpidMsb[4] = 0;
        cnmGenerationCfg.cpidMsb[5] = 0;
        cnmGenerationCfg.cpidMsb[6] = 0;
        cnmGenerationCfg.cnUntaggedEnable = GT_TRUE;
        cnmGenerationCfg.forceCnTag = GT_FALSE;
        cnmGenerationCfg.flowIdTag = 0;
        if(ingressPktHdr)
        {
            cnmGenerationCfg.appendPacket = GT_TRUE;
        }
        else
        {
            cnmGenerationCfg.appendPacket = GT_FALSE;
        }
        cnmGenerationCfg.keepQcnSrcInfo = GT_TRUE;

        rc = prvTgfPortCnMessageGenerationConfigSet(prvTgfDevNum, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigSet");

        /* AUTODOC: Save CN Profile Cfg for Drop Profile 7 for all TCs */
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortCnProfileQueueConfigGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                    tc,
                    &cnProfileCfgParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigGet");
        }

        /* AUTODOC: Configure limits and cn awareness for traffic class and profile */
        cpssOsMemSet(&cnProfileCfgParams, 0, sizeof(cnProfileCfgParams));
        cnProfileCfgParams.cnAware = GT_TRUE;
        cnProfileCfgParams.threshold = 1;

        cpssOsMemSet(&defCnProfileCfgParams, 0, sizeof(defCnProfileCfgParams));
        defCnProfileCfgParams.cnAware = GT_TRUE;
        /* make sure high threshold to not trigger CNM */
        defCnProfileCfgParams.threshold = PRV_TGF_QCN_COUNT_OF_FRAMES_TO_TRANSMIT_CNS + 100;

        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            /* Set CN Profile Cfg for Drop Profile 7 for all TCs exclude TC used on test */
            profileParamsPtr = (tc == PRV_TGF_TRAFFIC_CLASS_CNS)
                        ? cnProfileCfgParams
                        : defCnProfileCfgParams;

            rc = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc,
                    &profileParamsPtr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigSet");
        }

        /* AUTODOC: set vlan entry */
        vlanTags[0] = 1;
        vlanTags[1] = 1;

        vlanPortsArray[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
        vlanPortsArray[1] = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, vlanPortsArray,
            NULL, vlanTags, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_CNS);

        rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = cpssDxChBrgMcMemberDelete(prvTgfDevNum, 0xFFF, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgMcMemberDelete: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

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
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_QOS_PORT_NO_TRUST_E);

        /* AUTODOC: Save CoS entry */
        rc = prvTgfCosProfileEntryGet(prvTgfDevNum,
            PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Set CoS entry */
        cosProfileConfig = cosProfileRestore;
        cosProfileConfig.trafficClass = PRV_TGF_TRAFFIC_CLASS_CNS;
        rc = prvTgfCosProfileEntrySet(PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileConfig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Save the CNM Termination status on send port */
        rc = prvTgfPortCnTerminationEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &cnmTermStatusSendPortPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableGet");

        /* AUTODOC: Set the CNM status on a given send port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* AUTODOC: Save the CNM Termination status on CPU port */
        rc = prvTgfPortCnTerminationEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            &cnmTermStatusCPUPortPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableGet");

        /* AUTODOC: Set the CNM status on a given CPU port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    }
    else
    {
        /* AUTODOC: Restore the CNM status on a given CPU port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            cnmTermStatusCPUPortPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
           prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* AUTODOC: Restore the CNM status on a given send port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            cnmTermStatusSendPortPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
           prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

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

        /* AUTODOC: Restore CN Profile Cfg for all traffic class for profile 7*/
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc,  &cnProfileCfgParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigSet");
        }

        /* AUTODOC: Restore the CNM Generation Config */
        rc = prvTgfPortCnMessageGenerationConfigSet(prvTgfDevNum, &cnmGenerationCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigGet");

        /* AUTODOC: Restore receive port (1) Drop Profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            profileSetPtrRestore);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpSet %d %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]);

        /* AUTODOC: Restore the CNM Target Assignment Mode */
        rc = prvTgfPortCnQueueStatusModeEnableSet(prvTgfDevNum, enablePtrRestore,
             targetHwDevPtrRestore, targetPortPtrRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableGet %d %d %d",
            enablePtrRestore, targetHwDevPtrRestore, targetPortPtrRestore);

        /* AUTODOC: Restore the CN sample interval entry */
        for (entryIndex = 0; entryIndex < PRV_TGF_SAMPLE_TABLE_ENTRY_CNS; entryIndex++)
        {
            rc = prvTgfPortCnSampleEntrySet(prvTgfDevNum, entryIndex, &cnSampleEntryPtrRestore[entryIndex]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnSampleEntrySet");
        }

        /* AUTODOC: Restore the Fb calculation configuration */
        rc = prvTgfPortCnFbCalcConfigSet(prvTgfDevNum, &fbCalcCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigGet");

        /* AUTODOC: Restore the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeSet(prvTgfDevNum, etherTypePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeGet %d",
            etherTypePtrRestore);

        /* AUTODOC: Restore cn enable/disable mechanism */
        rc = prvTgfPortCnDbaModeEnableSet(prvTgfDevNum, cnModePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnDbaModeEnableGet: %d", cnModePtrRestore);

        /* AUTODOC: Restore VLAN related config  */
        rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = cpssDxChBrgMcMemberAdd(prvTgfDevNum, 0xFFF, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgMcMemberAdd: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

        /* AUTODOC: Reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS] = portForCpuRestore;
    }
}

/**
* @internal tgfPortTxQcnDbaQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxQcnDbaQueueResources test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxQcnDbaQueueResourcesConfigure
(
    GT_BOOL configSet
)
{
    GT_STATUS                                     rc = GT_OK;
    GT_U8                                         tc;
    GT_U8                                         entryIndex;
    static GT_U32                                 dbaAvailableBufferRestore;
    static CPSS_QOS_ENTRY_STC                     qosRecvPortConfigRestore;
    CPSS_QOS_ENTRY_STC                            qosRecvPortConfig;
    static PRV_TGF_COS_PROFILE_STC                cosProfileRestore;
    PRV_TGF_COS_PROFILE_STC                       cosProfileConfig;
    static CPSS_QOS_PORT_TRUST_MODE_ENT           trustModeRestore;
    GT_U8                                         vlanTags[2];
    GT_U32                                        vlanPortsArray[2];
    static CPSS_DXCH_PORT_CN_MODE_ENT             cnModePtrRestore;
    static GT_U32                                 etherTypePtrRestore;
    static CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfgPtrRestore;
    static CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC fbCalcCfg;
    static CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC cnSampleEntryPtrRestore[CPSS_TC_RANGE_CNS];
    static CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC cnSampleEntry;
    static GT_BOOL                                enablePtrRestore;
    static GT_HW_DEV_NUM                          targetHwDevPtrRestore;
    static GT_PORT_NUM                            targetPortPtrRestore;
    static GT_HW_DEV_NUM                          targetHwDev;
    static CPSS_PORT_TX_DROP_PROFILE_SET_ENT      profileSetPtrRestore;
    static CPSS_PORT_CNM_GENERATION_CONFIG_STC    cnmGenerationCfgPtrRestore;
    static CPSS_PORT_CNM_GENERATION_CONFIG_STC    cnmGenerationCfg;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        profileParamsPtr;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        cnProfileCfgParams;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        defCnProfileCfgParams;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        cnProfileCfgParamsRestore[CPSS_TC_RANGE_CNS];
    static GT_BOOL                                enableRestore;
    static GT_BOOL                                cnmTermStatusSendPortPtrRestore;
    static GT_BOOL                                cnmTermStatusCPUPortPtrRestore;
    static GT_U32                                 portForCpuRestore;

    if(configSet)
    {
        portForCpuRestore = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS] = PRV_TGF_QCN_CPU_PORT_CNS;

        /* AUTODOC: save cn enable/disable mechanism */
        rc = prvTgfPortCnModeEnableGet(prvTgfDevNum, &cnModePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnModeEnableGet: %d", cnModePtrRestore);

        /* AUTODOC: set cn enable mechanism */
        rc = prvTgfPortCnModeEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnModeEnableSet");

        /* AUTODOC: save qcn dba mode */
        rc = prvTgfPortCnDbaModeEnableGet(prvTgfDevNum, &enableRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnDbaModeEnableGet: %d", enableRestore);

        /* AUTODOC: set qcn dba mode */
        rc = prvTgfPortCnDbaModeEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaModeEnableSet");

       /* AUTODOC: Save number of available buffers for DBA QCN */
       rc = prvTgfPortCnDbaAvailableBuffGet(prvTgfDevNum,
           &dbaAvailableBufferRestore);
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaAvailableBuffGet");

       /* AUTODOC: Set number of available buffers for DBA QCN */
       rc = prvTgfPortCnDbaAvailableBuffSet(prvTgfDevNum,
           PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaAvailableBuffSet: %d",
           PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);

        /* AUTODOC: Save the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeGet(prvTgfDevNum, &etherTypePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeGet %d",
            etherTypePtrRestore);

        /* AUTODOC: Set the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeSet(prvTgfDevNum, 0x22E9);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeSet");

        /* AUTODOC: Save the Fb calculation configuration */
        rc = prvTgfPortCnFbCalcConfigGet(prvTgfDevNum, &fbCalcCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigGet");

        /* AUTODOC: Set the Fb calculation configuration */
        fbCalcCfg.wExp = 0;
        fbCalcCfg.fbLsb = 0;
        fbCalcCfg.deltaEnable = GT_TRUE;
        fbCalcCfg.fbMin = 0;
        fbCalcCfg.fbMax = 50;
        rc = prvTgfPortCnFbCalcConfigSet(prvTgfDevNum, &fbCalcCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigSet");

        /* AUTODOC: Save the CN sample interval entry */
        for (entryIndex = 0; entryIndex < PRV_TGF_SAMPLE_TABLE_ENTRY_CNS; entryIndex++)
        {
            rc = prvTgfPortCnSampleEntryGet(prvTgfDevNum, entryIndex, &cnSampleEntryPtrRestore[entryIndex]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnSampleEntryGet");
        }

        /* AUTODOC: Set the CN sample interval entry */
        cnSampleEntry.interval = PRV_TGF_CN_SAMPLE_INTERVAL_CNS / PRV_TGF_CN_SAMPLE_INTERVAL_GRANULARITY_CNS;
        cnSampleEntry.randBitmap = 0;
        for (entryIndex = 0; entryIndex < PRV_TGF_SAMPLE_TABLE_ENTRY_CNS; entryIndex++)
        {
            rc = prvTgfPortCnSampleEntrySet(prvTgfDevNum, entryIndex, &cnSampleEntry);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnSampleEntrySet");
        }

        /* AUTODOC: Save the CNM Target Assignment Mode */
        rc = prvTgfPortCnQueueStatusModeEnableGet(prvTgfDevNum, &enablePtrRestore,
            &targetHwDevPtrRestore, &targetPortPtrRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableGet %d %d %d",
            enablePtrRestore, targetHwDevPtrRestore, targetPortPtrRestore);

        /* AUTODOC: Set the CNM Target Assignment Mode */
        rc = extUtilHwDeviceNumberGet(prvTgfDevNum, &targetHwDev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "extUtilHwDeviceNumberGet");
        rc = prvTgfPortCnQueueStatusModeEnableSet(prvTgfDevNum, GT_TRUE,
            targetHwDev, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableSet");

        /* AUTODOC: Save the receive port (1) drop profile */
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], &profileSetPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet %d",
            profileSetPtrRestore);

        /* AUTODOC: Set the receive port (1) drop profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], PRV_TGF_TD_PROFILE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

        /* AUTODOC: Save the CNM Generation Config */
        rc = prvTgfPortCnMessageGenerationConfigGet(prvTgfDevNum, &cnmGenerationCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigGet");

        /* AUTODOC: Set the CNM Generation Config */
        cnmGenerationCfg.qosProfileId = 0;
        cnmGenerationCfg.isRouted = 0;
        cnmGenerationCfg.overrideUp = GT_TRUE;
        cnmGenerationCfg.cnmUp = 0;
        cnmGenerationCfg.defaultVlanId = 0;
        cnmGenerationCfg.scaleFactor = 256;
        cnmGenerationCfg.version = 0;
        cnmGenerationCfg.cpidMsb[0] = 0;
        cnmGenerationCfg.cpidMsb[1] = 0;
        cnmGenerationCfg.cpidMsb[2] = 0;
        cnmGenerationCfg.cpidMsb[3] = 0;
        cnmGenerationCfg.cpidMsb[4] = 0;
        cnmGenerationCfg.cpidMsb[5] = 0;
        cnmGenerationCfg.cpidMsb[6] = 0;
        cnmGenerationCfg.cnUntaggedEnable = GT_TRUE;
        cnmGenerationCfg.forceCnTag = GT_FALSE;
        cnmGenerationCfg.flowIdTag = 0;
        cnmGenerationCfg.appendPacket = GT_TRUE;
        cnmGenerationCfg.keepQcnSrcInfo = GT_TRUE;

        rc = prvTgfPortCnMessageGenerationConfigSet(prvTgfDevNum, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigSet");

        /* AUTODOC: Save CN Profile Cfg for Drop Profile 7 for all TCs */
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortCnProfileQueueConfigGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                    tc,
                    &cnProfileCfgParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigGet");
        }

        /* AUTODOC: Configure limits and cn awareness for traffic class and profile */
        cpssOsMemSet(&cnProfileCfgParams, 0, sizeof(cnProfileCfgParams));
        cnProfileCfgParams.cnAware = GT_TRUE;
        cnProfileCfgParams.threshold = prvTgfQcnDbaQueueLimits[5];
        cnProfileCfgParams.alpha = prvTgfQcnDbaQueueAlpha[5];

        cpssOsMemSet(&defCnProfileCfgParams, 0, sizeof(defCnProfileCfgParams));
        defCnProfileCfgParams.cnAware = GT_TRUE;
        defCnProfileCfgParams.threshold = prvTgfQcnDbaQueueLimits[0] + 100;
        defCnProfileCfgParams.alpha = prvTgfQcnDbaQueueAlpha[0];

        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            /* Set CN Profile Cfg for Drop Profile 7 for all TCs exclude TC used on test */
            profileParamsPtr = (tc == PRV_TGF_TRAFFIC_CLASS_CNS)
                        ? cnProfileCfgParams
                        : defCnProfileCfgParams;

            rc = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc, &profileParamsPtr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigSet");
        }

        /* AUTODOC: set vlan entry */
        vlanTags[0] = 1;
        vlanTags[1] = 1;
        vlanPortsArray[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
        vlanPortsArray[1] = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, vlanPortsArray,
            NULL, vlanTags, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_CNS);

        rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = cpssDxChBrgMcMemberDelete(prvTgfDevNum, 0xFFF, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgMcMemberDelete: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

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
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_QOS_PORT_NO_TRUST_E);

        /* AUTODOC: Save CoS entry */
        rc = prvTgfCosProfileEntryGet(prvTgfDevNum,
            PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Set CoS entry */
        cosProfileConfig = cosProfileRestore;
        cosProfileConfig.trafficClass = PRV_TGF_TRAFFIC_CLASS_CNS;
        rc = prvTgfCosProfileEntrySet(PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileConfig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Save the CNM Termination status on send port */
        rc = prvTgfPortCnTerminationEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &cnmTermStatusSendPortPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableGet");

        /* AUTODOC: Set the CNM status on a given send port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* AUTODOC: Save the CNM Termination status on CPU port */
        rc = prvTgfPortCnTerminationEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            &cnmTermStatusCPUPortPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableGet");

        /* AUTODOC: Set the CNM status on a given CPU port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    }
    else
    {
        /* AUTODOC: Restore the CNM status on a given CPU port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            cnmTermStatusCPUPortPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
           prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* AUTODOC: Restore the CNM status on a given send port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            cnmTermStatusSendPortPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
           prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

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

        /* AUTODOC: Restore CN Profile Cfg for all traffic class for profile 7*/
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc,  &cnProfileCfgParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigSet");
        }

        /* AUTODOC: Restore the CNM Generation Config */
        rc = prvTgfPortCnMessageGenerationConfigSet(prvTgfDevNum, &cnmGenerationCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigGet");

        /* AUTODOC: Restore receive port (1) Drop Profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            profileSetPtrRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpSet %d %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], profileSetPtrRestore);

        /* AUTODOC: Restore the CNM Target Assignment Mode */
        rc = prvTgfPortCnQueueStatusModeEnableSet(prvTgfDevNum, enablePtrRestore,
             targetHwDevPtrRestore, targetPortPtrRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableGet %d %d %d",
            enablePtrRestore, targetHwDevPtrRestore, targetPortPtrRestore);

        /* AUTODOC: Restore the CN sample interval entry */
        for (entryIndex = 0; entryIndex < PRV_TGF_SAMPLE_TABLE_ENTRY_CNS; entryIndex++)
        {
            rc = prvTgfPortCnSampleEntrySet(prvTgfDevNum, entryIndex, &cnSampleEntryPtrRestore[entryIndex]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnSampleEntrySet");
        }

        /* AUTODOC: Restore the Fb calculation configuration */
        rc = prvTgfPortCnFbCalcConfigSet(prvTgfDevNum, &fbCalcCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigGet");

        /* AUTODOC: Restore the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeSet(prvTgfDevNum, etherTypePtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeGet");

        /* AUTODOC: Restore qcn dba mode */
        rc = prvTgfPortCnDbaModeEnableSet(prvTgfDevNum, enableRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnDbaModeEnableGet: %d", enableRestore);

        /* AUTODOC: Restore cn enable/disable mechanism */
        rc = prvTgfPortCnModeEnableSet(prvTgfDevNum, cnModePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnModeEnableGet: %d", cnModePtrRestore);

        /* AUTODOC: Restore VLAN related config  */
        rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = cpssDxChBrgMcMemberAdd(prvTgfDevNum, 0xFFF, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgMcMemberAdd: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

        /* AUTODOC: Reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS] = portForCpuRestore;
    }
}

/**
* @internal tgfPortTxQcnPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic for tgfPortTxQcnQueueResources test.
*
*/
GT_VOID tgfPortTxQcnPortResourcesTrafficGenerate
(
    GT_BOOL ingressPktHdr
)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                  buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                  packetActualLength = 0;
    GT_U8                   queue;
    GT_U8                   devNum = prvTgfDevNum;
    GT_HW_DEV_NUM           hwDevNum;
    CPSS_DXCH_NET_RX_PARAMS_STC dxChPcktParams;
    GT_U32                  numOfCpuPackets = 0;
    GT_BOOL                 encapsulatedSDU = GT_FALSE;
    GT_BOOL                 dstMacCheck = GT_TRUE;
    GT_BOOL                 payloadCheck = GT_TRUE;
    GT_U16                  SDULength;
    GT_U16                  qcnSduOffset;
    GT_U16                  sduLenOffset;
    GT_U16                  encapMacDaOffset;
    GT_U16                  vlanVidStart;
    GT_U16                  payloadStart;
    GT_U16                  vlanStart;
    GT_U8                   pri;
    GT_U8                   cfi;
    GT_U16                  vid;
    TGF_ETHER_TYPE          etherType;

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    /* AUTODOC: Block TxQ (7) for receive port (1).*/
    st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
        PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
        PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);

    /* clear table */
    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    st = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: Transmit 10 frames (with MAC DA 00:01:02:03:AA:01) to
       port 0. */
    st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
        PRV_TGF_QCN_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
        PRV_TGF_QCN_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

    st = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
        "ERROR of StartTransmitting: %d, %d",
        prvTgfDevNum, 1);

    cpssOsTimerWkAfter(1);

    prvTgfPrintPortCountersEth();

    /* AUTODOC: Enable TxQ (7) for receive port (1).*/
    st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
        PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
        PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);

    prvTgfPrintPortCountersEth();

    /* stop Rx capture */
    st = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, st, "tgfTrafficTableRxStartCapture");

    st = extUtilHwDeviceNumberGet(devNum, &hwDevNum);
    if(st != GT_OK)
    {
      UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
      return;
    }

    do
    {
        st = tgfTrafficTableRxPcktGet(numOfCpuPackets++, packetBuff, &buffLen, &packetActualLength,
            &devNum, &queue, (GT_VOID *)&dxChPcktParams);
        if (numOfCpuPackets < 2)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktGet");
        }

        if (st == GT_OK)
        {
            PRV_UTF_LOG1_MAC(" QCN Packet [%d] in CPU\n", numOfCpuPackets);
            tgfTrafficTracePacket(packetBuff, buffLen, GT_TRUE);

            UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.commonParams.dsaTagType,
                CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT, "dsaTagType different than expected which is extended DSA tag - 4 words");

            UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.dsaType,
                CPSS_DXCH_NET_DSA_CMD_FORWARD_E, "dsaType is different than expected which is FORWARD");

            UTF_VERIFY_EQUAL0_STRING_MAC(hwDevNum,
                dxChPcktParams.dsaParam.dsaInfo.forward.srcHwDev,
                "sampled packet dev num different than expected");

            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                dxChPcktParams.dsaParam.dsaInfo.forward.source.portNum,
                "sampled packet port num different than expected");

            UTF_VERIFY_EQUAL0_STRING_MAC(hwDevNum,
                dxChPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum,
                "qcn pkt target dev num different than expected");

            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                dxChPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum,
                "qcn pkt target port num different than expected");

            /* QCN pkt SDU length decides if IngressPktHdr is part of QCN. SDU
             * length is at offset 41-42 in QCN frame which is part of QCN Hdr.
             * [DA(6)+SA(6)+VLAN(4)+CNEtherType(2)+QCNHdr(24)]
             */
            qcnSduOffset = TGF_QCN_DA_CNS + TGF_QCN_SA_CNS + TGF_QCN_VLAN_CNS + TGF_QCN_ETHER_TYPE_CNS + TGF_QCN_HDR_CNS;
            sduLenOffset = qcnSduOffset - TGF_QCN_HDR_SDU_LENGTH_CNS;
            encapMacDaOffset = sduLenOffset - TGF_QCN_DA_CNS;
            SDULength = (GT_U16)(packetBuff[sduLenOffset + 1] | ((packetBuff[sduLenOffset] & 0xFF) << 8));
            encapsulatedSDU = (SDULength > 0) ? 1 : 0;
            if (ingressPktHdr)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(1, encapsulatedSDU,
                    "encapsulatedSDULLength zero and expected is non-zero");

                /* Encapsulated MAC DA check */
                dstMacCheck = cpssOsMemCmp(&(prvTgfPacketL2Part.daMac), &packetBuff[encapMacDaOffset], sizeof(TGF_QCN_DA_CNS));
                UTF_VERIFY_EQUAL0_STRING_MAC(0, dstMacCheck,
                    "dstMac os sampled pkt different than expected");

                /* Vlan Check of the SDU [TPID, CFI, UP, VID] */
                vlanStart = qcnSduOffset;
                vlanVidStart = qcnSduOffset + TGF_QCN_VLAN_TPID_LENGTH_CNS;

                etherType = (TGF_ETHER_TYPE) (packetBuff[vlanStart + 1] | (packetBuff[vlanStart] << 8));
                vid = (GT_U16)(packetBuff[vlanVidStart + 1] | ((packetBuff[vlanVidStart] & 0x0F) << 8));
                pri = (GT_U8)((packetBuff[vlanVidStart] >> 5) & 0x7);
                cfi = (GT_U8)((packetBuff[vlanVidStart] >> 4) & 0x1);

                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.etherType, etherType,
                    "TPID of sampled pkt different than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.pri, pri,
                    "vlanPri of sampled pkt different than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.cfi, cfi,
                    "vlanCfi of sampled pkt different than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.vid, vid,
                    "vlanVid of sampled pkt different than expected");

                /* Payload check of the SDU, first 20 bytes */
                payloadStart = qcnSduOffset + TGF_QCN_VLAN_CNS;
                payloadCheck = cpssOsMemCmp(prvTgfPayloadDataArr,&packetBuff[payloadStart], 20);
                UTF_VERIFY_EQUAL0_STRING_MAC(0, payloadCheck,
                    "ingressHdrInQCN bytes different than payload sent");
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(0,
                    encapsulatedSDU,
                    "encapsulatedSDULLength non-zero and expected is zero");
            }

        }
    } while (st == GT_OK);

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal tgfPortTxQcnDbaPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic for tgfPortTxQcnDbaQueueResources test.
*
*/
GT_VOID tgfPortTxQcnDbaPortResourcesTrafficGenerate
(
    GT_BOOL ingressPktHdr
)
{
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                                        buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                                        packetActualLength = 0;
    GT_U32                                        ii;
    GT_U8                                         queue;
    GT_U8                                         devNum = prvTgfDevNum;
    GT_HW_DEV_NUM                                 hwDevNum;
    CPSS_DXCH_NET_RX_PARAMS_STC                   dxChPcktParams;
    GT_U32                                        numOfCpuPackets = 0;
    GT_BOOL                                       encapsulatedSDU = GT_FALSE;
    GT_BOOL                                       dstMacCheck = GT_TRUE;
    GT_BOOL                                       payloadCheck = GT_TRUE;
    GT_BOOL                                       expectedSduLengthCheck = 0;
    GT_U16                                        SDULength;
    GT_U16                                        qcnSduOffset;
    GT_U16                                        sduLenOffset;
    GT_U16                                        encapMacDaOffset;
    GT_U16                                        vlanVidStart;
    GT_U16                                        payloadStart;
    GT_U16                                        vlanStart;
    GT_U8                                         pri;
    GT_U8                                         cfi;
    GT_U16                                        vid;
    TGF_ETHER_TYPE                                etherType;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT          alpha;
    GT_U32                                        expectedValue = 0;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        cnProfileCfgParams;

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    /* AUTODOC: Go over all alphas */
    for (ii = 0; ii < PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS; ii++)
    {
        alpha = prvTgfQcnDbaQueueAlpha[ii];

        PRV_UTF_LOG1_MAC("ALPHA:   %s\n",
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E)     ? "0.0"   :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E)  ? "0.25"  :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E)   ? "0.5"   :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E)     ? "1.0"   :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E)     ? "2.0"   :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E)     ? "4.0"   :
            "BAD PARAMETER");

        /* AUTODOC: Configure qcn profile config including alpha for traffic class 7 and drop profile 7*/
        cpssOsMemSet(&cnProfileCfgParams, 0, sizeof(cnProfileCfgParams));
        cnProfileCfgParams.cnAware = GT_TRUE;
        cnProfileCfgParams.threshold = prvTgfQcnDbaQueueLimits[5];
        cnProfileCfgParams.alpha = alpha;

        st = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum,
            PRV_TGF_TD_PROFILE_CNS, PRV_TGF_TRAFFIC_CLASS_CNS, &cnProfileCfgParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfPortCnProfileQueueConfigSet");

        /* Get expected amount of available buffers */
        st = prvTgfPortTxQcnDbaQueueExpectedAmountOfBuffers(ii,
            &expectedValue);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQcnDbaQueueExpectedAmountOfBuffers");

        /* AUTODOC: Block TxQ (7) for receive port (1).*/
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);

        /* clear table */
        st = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

        /* start Rx capture */
        st = tgfTrafficTableRxStartCapture(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

        /* AUTODOC: Transmit 100 frames (with MAC DA 00:01:02:03:AA:01) to
           port 0. */
        st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
            PRV_TGF_QCN_COUNT_OF_FRAMES_DBA_QCN_TO_TRANSMIT_CNS, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
            PRV_TGF_QCN_COUNT_OF_FRAMES_DBA_QCN_TO_TRANSMIT_CNS, 0, NULL);

        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, 1);

        cpssOsTimerWkAfter(1);

        prvTgfPrintPortCountersEth();

        /* AUTODOC: Enable TxQ (7) for receive port (1).*/
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);

        prvTgfPrintPortCountersEth();

        /* stop Rx capture */
        st = tgfTrafficTableRxStartCapture(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "tgfTrafficTableRxStartCapture");

        st = extUtilHwDeviceNumberGet(devNum, &hwDevNum);
        if(st != GT_OK)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            return;
        }
        numOfCpuPackets = 0;
        do
        {
            st = tgfTrafficTableRxPcktGet(numOfCpuPackets++, packetBuff, &buffLen, &packetActualLength,
                &devNum, &queue, (GT_VOID *)&dxChPcktParams);
            if ((alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E && numOfCpuPackets < expectedValue))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktGet");
            }

            if (st == GT_OK)
            {
                PRV_UTF_LOG1_MAC(" QCN Packet [%d] in CPU\n", numOfCpuPackets);
                tgfTrafficTracePacket(packetBuff, buffLen, GT_TRUE);

                UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.commonParams.dsaTagType,
                    CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT, "dsaTagType different than expected which is extended DSA tag - 4 words");

                UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.dsaType,
                    CPSS_DXCH_NET_DSA_CMD_FORWARD_E, "dsaType is different than expected which is FORWARD");

                UTF_VERIFY_EQUAL0_STRING_MAC(hwDevNum,
                    dxChPcktParams.dsaParam.dsaInfo.forward.srcHwDev,
                    "sampled packet dev num different than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    dxChPcktParams.dsaParam.dsaInfo.forward.source.portNum,
                    "sampled packet port num different than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(hwDevNum,
                    dxChPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum,
                    "qcn pkt target dev num different than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                    dxChPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum,
                    "qcn pkt target port num different than expected");

                /* QCN pkt SDU length decides if IngressPktHdr is part of QCN. SDU
                 * length is at offset 41-42 in QCN frame which is part of QCN Hdr.
                 * [DA(6)+SA(6)+VLAN(4)+CNEtherType(2)+QCNHdr(24)]
                 */
                qcnSduOffset = TGF_QCN_DA_CNS + TGF_QCN_SA_CNS + TGF_QCN_VLAN_CNS + TGF_QCN_ETHER_TYPE_CNS + TGF_QCN_HDR_CNS;
                sduLenOffset = qcnSduOffset - TGF_QCN_HDR_SDU_LENGTH_CNS;
                encapMacDaOffset = sduLenOffset - TGF_QCN_DA_CNS;
                SDULength = (GT_U16)(packetBuff[sduLenOffset + 1] | ((packetBuff[sduLenOffset] & 0xFF) << 8));
                encapsulatedSDU = (SDULength > 0) ? 1 : 0;
                if (ingressPktHdr)
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(1, encapsulatedSDU,
                        "encapsulatedSDULLength zero and expected is non-zero");

                    /* Encapsulated SDU can be max 86 bytes */
                    if (encapsulatedSDU && SDULength <= 86)
                    {
                        expectedSduLengthCheck = 1;
                    }
                    UTF_VERIFY_EQUAL1_STRING_MAC(1, expectedSduLengthCheck,
                        "encapsulatedSDU greater than expected %d", SDULength);

                    /* Encapsulated MAC DA check */
                    dstMacCheck = cpssOsMemCmp(&(prvTgfPacketL2Part.daMac), &packetBuff[encapMacDaOffset], sizeof(TGF_QCN_DA_CNS));
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, dstMacCheck,
                        "dstMac os sampled pkt different than expected");

                    /* Vlan Check of the SDU [TPID, CFI, UP, VID] */
                    vlanStart = qcnSduOffset;
                    vlanVidStart = qcnSduOffset + TGF_QCN_VLAN_TPID_LENGTH_CNS;

                    etherType = (TGF_ETHER_TYPE) (packetBuff[vlanStart + 1] | (packetBuff[vlanStart] << 8));
                    vid = (GT_U16)(packetBuff[vlanVidStart + 1] | ((packetBuff[vlanVidStart] & 0x0F) << 8));
                    pri = (GT_U8)((packetBuff[vlanVidStart] >> 5) & 0x7);
                    cfi = (GT_U8)((packetBuff[vlanVidStart] >> 4) & 0x1);
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.etherType, etherType,
                        "TPID of sampled pkt different than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.pri, pri,
                        "vlanPri of sampled pkt different than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.cfi, cfi,
                        "vlanCfi of sampled pkt different than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.vid, vid,
                        "vlanVid of sampled pkt different than expected");

                    /* Payload check of the SDU, first 20 bytes */
                    payloadStart = qcnSduOffset + TGF_QCN_VLAN_CNS;
                    payloadCheck = cpssOsMemCmp(prvTgfPayloadDataArr,&packetBuff[payloadStart], 20);
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, payloadCheck,
                        "ingressHdrInQCN bytes different than payload sent");
                }
                else
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(0,
                        encapsulatedSDU,
                        "encapsulatedSDULLength non-zero and expected is zero");
                }
            }
        } while (st == GT_OK);

        /* exclude GT_NOT_FOUND from calculation */
        --numOfCpuPackets;

        UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, numOfCpuPackets,
            "number of QCN packets different than expected");

        st = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");
    }
}

/**
* @internal tgfPortTxSip6QcnQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxSip6QcnQueueResources test configuration
*
* @param[in] configSet       - store/restore configuration
*                              GT_TRUE  -- configure
*                              GT_FALSE -- restore
* @param[in] ingressPktHdr   - include/exclude the incoming packet header
*                              GT_TRUE -- include incoming packet header
*                              GT_FALSE -- esclude incoming pakcet header 
* @param[in] resource        - specifies the global/pool available buffer limit
*                              for QCN DBA
*                              CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E/
*                              CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E
* @param[in] poolNum         - specifies the pool number 0/1
* @param[in] queuePortLimits - specifies the QCN is triggered based on
*                              queue/port profile limits
*                              PRV_TGF_PORT_CN_QUEUE_LIMIT_E/
*                              PRV_TGF_PORT_CN_PORT_LIMIT_E
*/
GT_VOID tgfPortTxSip6QcnQueueResourcesConfigure
(
    GT_BOOL configSet,
    GT_BOOL ingressPktHdr,
    GT_U32  resource,
    GT_U32  poolNum,
    PRV_TGF_PORT_CN_LIMIT_ENT  queuePortLimits
)
{
    GT_STATUS                                                  rc = GT_OK;
    GT_U8                                                      tc;
    static CPSS_QOS_ENTRY_STC                                  qosRecvPortConfigRestore;
    CPSS_QOS_ENTRY_STC                                         qosRecvPortConfig;
    static PRV_TGF_COS_PROFILE_STC                             cosProfileRestore;
    PRV_TGF_COS_PROFILE_STC                                    cosProfileConfig;
    static CPSS_QOS_PORT_TRUST_MODE_ENT                        trustModeRestore;
    GT_U8                                                      vlanTags[2];
    GT_U32                                                     vlanPortsArray[2];
    static CPSS_DXCH_PORT_CN_MODE_ENT                          cnModePtrRestore;
    static GT_U32                                              etherTypePtrRestore;
    static CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC              fbCalcCfgPtrRestore;
    static CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC              fbCalcCfg;
    static GT_BOOL                                             enablePtrRestore;
    static GT_HW_DEV_NUM                                       targetHwDevPtrRestore;
    static GT_PORT_NUM                                         targetPortPtrRestore;
    static GT_HW_DEV_NUM                                       targetHwDev;
    static CPSS_PORT_TX_DROP_PROFILE_SET_ENT                   profileSetPtrRestore;
    static CPSS_PORT_CNM_GENERATION_CONFIG_STC                 cnmGenerationCfgPtrRestore;
    static CPSS_PORT_CNM_GENERATION_CONFIG_STC                 cnmGenerationCfg;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC                     cnProfileCfgParams;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC                     defCnProfileCfgParams;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC                     profileParamsPtr;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC                     cnProfileCfgParamsRestore[CPSS_SIP6_TC_RANGE_CNS];
    static GT_BOOL                                             cnmTermStatusSendPortPtrRestore;
    static GT_BOOL                                             cnmTermStatusCPUPortPtrRestore;
    static GT_U32                                              portForCpuRestore;
    static CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocation;
    static CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT congestedQPriorityLocationRestore;
    static GT_BOOL                                             enableRestore;
    static GT_BOOL                                             enable;
    static GT_U32                                              index;
    static CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC                qcnTriggerPtrRestore;
    static CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC                qcnTriggerPtr;
    static CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT            resourceModeRestore;
    static CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT            resourceMode;
    static GT_U32                                              dbaAvailableBufferRestore;
    static GT_U32                                              globalAvailableBuffersRestore;
    static GT_U32                                              pool0AvailableBuffersRestore;
    static GT_U32                                              pool1AvailableBuffersRestore;
    static GT_U32                                              globalAvailableBuffers;
    static GT_U32                                              pool0AvailableBuffers;
    static GT_U32                                              pool1AvailableBuffers;
    static GT_U32                                              poolAvailableBuffRestore;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC                     cnPortProfileCfgParams;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC                     cnPortProfileCfgParamsRestore;

    if(configSet)
    {
        portForCpuRestore = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS] = PRV_TGF_QCN_CPU_PORT_CNS;

        /* AUTODOC: save configured resource mode for QCN DBA */
        rc = prvTgfPortTxTailDropGlobalParamsGet(prvTgfDevNum, &resourceModeRestore,
             &globalAvailableBuffersRestore, &pool0AvailableBuffersRestore, &pool1AvailableBuffersRestore);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
            "cpssDxChPortTxTailDropGlobalParamsGet: %d %d %d %d", resourceModeRestore, globalAvailableBuffersRestore,
            pool0AvailableBuffersRestore, pool1AvailableBuffersRestore);

        /* AUTODOC: set resource mode for QCN DBA [global/pool] */
        /* The api sets the free buffers for global and pool resource mode
         * along with the resource mode. The buffers configured are dummy
         * and not used for QCN DBA tests. The buffers are instead configured
         * using different set of apis called from PortCn module.
         */
        globalAvailableBuffers = 10;
        pool0AvailableBuffers = 10;
        pool1AvailableBuffers = 10;
        rc = prvTgfPortTxTailDropGlobalParamsSet(prvTgfDevNum, resourceMode, globalAvailableBuffers,
             pool0AvailableBuffers, pool1AvailableBuffers);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPortTxTailDropGlobalParamsSet %d %d %d %d", resourceMode,
             globalAvailableBuffers, pool0AvailableBuffers, pool1AvailableBuffers);

        /* AUTODOC: save cn enable/disable mechanism */
        rc = prvTgfPortCnModeEnableGet(prvTgfDevNum, &cnModePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnModeEnableGet: %d", cnModePtrRestore);

        /* AUTODOC: set cn enable mechanism */
        rc = prvTgfPortCnModeEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnModeEnableSet");

        /* AUTODOC: Save the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeGet(prvTgfDevNum, &etherTypePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeGet %d",
            etherTypePtrRestore);

        /* AUTODOC: Set the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeSet(prvTgfDevNum, 0x22E9);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeSet");

        if(resource == CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E)
        {
            /* AUTODOC: Save number of global available buffers for DBA QCN */
            rc = prvTgfPortCnDbaAvailableBuffGet(prvTgfDevNum,
                &dbaAvailableBufferRestore);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaAvailableBuffGet");

            /* AUTODOC: Set number of global available buffers for DBA QCN */
            rc = prvTgfPortCnDbaAvailableBuffSet(prvTgfDevNum,
                PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaAvailableBuffSet: %d",
                PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
        }
        else
        {
            /* AUTODOC: Save number of pool available buffers for DBA QCN */
            rc = prvTgfPortCnDbaPoolAvailableBuffGet(prvTgfDevNum,
                poolNum, &poolAvailableBuffRestore);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaPoolAvailableBuffGet");

            /* AUTODOC: Set number of pool 0/1 available buffers for DBA QCN */
            rc = prvTgfPortCnDbaPoolAvailableBuffSet(prvTgfDevNum,
                poolNum, PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaPoolAvailableBuffSet: %d",
                PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
        }

       /* AUTODOC: Save congested q/priority bits in CNM header */
       rc = prvTgfPortCnCongestedQPriorityLocationGet(prvTgfDevNum,
           &congestedQPriorityLocationRestore);
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnCongestedQPriorityLocationGet");

       /* AUTODOC: Set congested q/priority bits in CNM header */
       congestedQPriorityLocation = CPSS_DXCH_PORT_CN_CONG_Q_NUM_E;
       rc = prvTgfPortCnCongestedQPriorityLocationSet(prvTgfDevNum,
           congestedQPriorityLocation);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnCongestedQPriorityLocationSet: %d",
           congestedQPriorityLocation);

       /* AUTODOC: Save CNM enable state for CN untagged frames */
       rc = prvTgfPortCnCNMEnableGet(prvTgfDevNum,
           &enableRestore);
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnCNMEnableGet");

       /* AUTODOC: Set CNM enable state for CN untagged frames */
       enable = GT_TRUE;;
       rc = prvTgfPortCnCNMEnableSet(prvTgfDevNum,
           enable);
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnCNMEnableSet: %d",
           enable);

       /* AUTODOC: Save QCN trigger threshold */
       index = 0;
       rc = prvTgfPortCnQcnTriggerGet(prvTgfDevNum, index,
           &qcnTriggerPtrRestore);
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnQcnTriggerGet");

       /* AUTODOC: Set QCN trigger threshold */
       /* Trigger probability is compared against the random number generated
        * by the PRNG HW module and bits 16:29 of the result are used for
        * comparison.
        */
       qcnTriggerPtr.qcnTriggerTh0 = 1;
       qcnTriggerPtr.qcnTriggerTh1 = 1;
       rc = prvTgfPortCnQcnTriggerSet(prvTgfDevNum, index,
           &qcnTriggerPtr);
       UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortCnQcnTriggerSet: index %d trigger 0 %d trigger 1 %d",
           index, qcnTriggerPtr.qcnTriggerTh0, qcnTriggerPtr.qcnTriggerTh1);

        /* AUTODOC: Save the Fb calculation configuration */
        rc = prvTgfPortCnFbCalcConfigGet(prvTgfDevNum, &fbCalcCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigGet %p");

        /* AUTODOC: Set the Fb calculation configuration */
        fbCalcCfg.fbLsb = 0;
        fbCalcCfg.fbMin = 0;
        fbCalcCfg.fbMax = 50;
        rc = prvTgfPortCnFbCalcConfigSet(prvTgfDevNum, &fbCalcCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigSet");

        /* AUTODOC: Save the CNM Target Assignment Mode */
        rc = prvTgfPortCnQueueStatusModeEnableGet(prvTgfDevNum, &enablePtrRestore,
            &targetHwDevPtrRestore, &targetPortPtrRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableGet %d %d %d",
            enablePtrRestore, targetHwDevPtrRestore, targetPortPtrRestore);

        /* AUTODOC: Set the CNM Target Assignment Mode */
        rc = extUtilHwDeviceNumberGet(prvTgfDevNum, &targetHwDev);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "extUtilHwDeviceNumberGet");
        rc = prvTgfPortCnQueueStatusModeEnableSet(prvTgfDevNum, GT_TRUE,
            targetHwDev, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableSet");

        /* AUTODOC: Save the receive port (1) drop profile */
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], &profileSetPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet %d",
            profileSetPtrRestore);

        /* AUTODOC: Set the receive port (1) drop profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], PRV_TGF_TD_PROFILE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");

        /* AUTODOC: Save the CNM Generation Config */
        rc = prvTgfPortCnMessageGenerationConfigGet(prvTgfDevNum, &cnmGenerationCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigGet");

        /* AUTODOC: Set the CNM Generation Config */
        cnmGenerationCfg.qosProfileId = 0;
        cnmGenerationCfg.isRouted = 0;
        cnmGenerationCfg.overrideUp = GT_TRUE;
        cnmGenerationCfg.cnmUp = 0;
        cnmGenerationCfg.defaultVlanId = 0;
        cnmGenerationCfg.scaleFactor = 256;
        cnmGenerationCfg.version = 0;
        cnmGenerationCfg.cpidMsb[0] = 0;
        cnmGenerationCfg.cpidMsb[1] = 0;
        cnmGenerationCfg.cpidMsb[2] = 0;
        cnmGenerationCfg.cpidMsb[3] = 0;
        cnmGenerationCfg.cpidMsb[4] = 0;
        cnmGenerationCfg.cpidMsb[5] = 0;
        cnmGenerationCfg.cpidMsb[6] = 0;
        cnmGenerationCfg.cnUntaggedEnable = GT_TRUE;
        cnmGenerationCfg.forceCnTag = GT_FALSE;
        cnmGenerationCfg.flowIdTag = 0;
        cnmGenerationCfg.qcnQOffset = 0;
        cnmGenerationCfg.qcnQDelta = 0;
        if(ingressPktHdr)
        {
            cnmGenerationCfg.appendPacket = GT_TRUE;
        }
        else
        {
            cnmGenerationCfg.appendPacket = GT_FALSE;
        }

        rc = prvTgfPortCnMessageGenerationConfigSet(prvTgfDevNum, &cnmGenerationCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigSet");

        if(queuePortLimits == PRV_TGF_PORT_CN_QUEUE_LIMIT_E ||
           queuePortLimits == PRV_TGF_PORT_CN_QUEUE_AND_PORT_LIMIT_E)
        {
            /* AUTODOC: Save CN Profile Cfg for Drop Profile 7 for all TCs 0..15 */
            for (tc = 0; tc < CPSS_SIP6_TC_RANGE_CNS; tc++)
            {
                rc = prvTgfPortCnProfileQueueConfigGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                                                       tc, &cnProfileCfgParamsRestore[tc]);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigGet");
            }

            /* AUTODOC: Configure limits and cn awareness for traffic class and profile */
            cpssOsMemSet(&cnProfileCfgParams, 0, sizeof(cnProfileCfgParams));
            cnProfileCfgParams.cnAware = GT_TRUE;
            cnProfileCfgParams.threshold = 1;

            cpssOsMemSet(&defCnProfileCfgParams, 0, sizeof(defCnProfileCfgParams));
            defCnProfileCfgParams.cnAware = GT_TRUE;
            /* high threshold configured for other TCs except the test TC to not trigger CNM */
            defCnProfileCfgParams.threshold = PRV_TGF_QCN_COUNT_OF_FRAMES_TO_TRANSMIT_CNS + 100;

            for (tc = 0; tc < CPSS_SIP6_TC_RANGE_CNS; tc++)
            {
                /* Set CN Profile Cfg for Drop Profile 7 for all TCs exclude TC used on test */
                profileParamsPtr = (tc == PRV_TGF_SIP6_TRAFFIC_CLASS_CNS)
                                   ? cnProfileCfgParams
                                   : defCnProfileCfgParams;

                rc = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS, tc,
                                                       &profileParamsPtr);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigSet");
            }
        }
        if(queuePortLimits == PRV_TGF_PORT_CN_PORT_LIMIT_E ||
           queuePortLimits == PRV_TGF_PORT_CN_QUEUE_AND_PORT_LIMIT_E)
        {
            /* AUTODOC: Save CN Profile Cfg for Drop Profile 7 for receiver port 1 */
            rc = prvTgfPortCnProfilePortConfigGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                                                  &cnPortProfileCfgParamsRestore);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfilePortConfigGet");

            /* AUTODOC: Configure limits and cn awareness for drop profile 7 */
            cpssOsMemSet(&cnPortProfileCfgParams, 0, sizeof(cnPortProfileCfgParams));
            cnPortProfileCfgParams.cnAware = GT_TRUE;
            cnPortProfileCfgParams.threshold = 1;

            rc = prvTgfPortCnProfilePortConfigSet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                                                  &cnPortProfileCfgParams);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfilePortConfigSet");
        }

        /* AUTODOC: set vlan entry */
        vlanTags[0] = 1;
        vlanTags[1] = 1;

        vlanPortsArray[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
        vlanPortsArray[1] = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, vlanPortsArray,
            NULL, vlanTags, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_CNS);

        rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = cpssDxChBrgMcMemberDelete(prvTgfDevNum, 0xFFF, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgMcMemberDelete: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

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
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_QOS_PORT_NO_TRUST_E);

        /* AUTODOC: Save CoS entry */
        rc = prvTgfCosProfileEntryGet(prvTgfDevNum,
            PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Set CoS entry */
        cosProfileConfig = cosProfileRestore;
        cosProfileConfig.trafficClass = PRV_TGF_SIP6_TRAFFIC_CLASS_CNS;
        rc = prvTgfCosProfileEntrySet(PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfileConfig);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Save the CNM Termination status on send port */
        rc = prvTgfPortCnTerminationEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &cnmTermStatusSendPortPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableGet");

        /* AUTODOC: Set the CNM status on a given send port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* AUTODOC: Save the CNM Termination status on CPU port */
        rc = prvTgfPortCnTerminationEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            &cnmTermStatusCPUPortPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableGet");

        /* AUTODOC: Set the CNM status on a given CPU port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    }
    else
    {
        /* AUTODOC: Restore the CNM status on a given CPU port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
            cnmTermStatusCPUPortPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
           prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* AUTODOC: Restore the CNM status on a given send port */
        rc = prvTgfPortCnTerminationEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            cnmTermStatusSendPortPtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnTerminationEnableSet %d",
           prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

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

        /* AUTODOC: Restore CN Profile Cfg for all traffic class for profile 7*/
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc,  &cnProfileCfgParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnProfileQueueConfigSet");
        }

        /* AUTODOC: Restore the CNM Generation Config */
        rc = prvTgfPortCnMessageGenerationConfigSet(prvTgfDevNum, &cnmGenerationCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnMessageGenerationConfigGet");

        /* AUTODOC: Restore receive port (1) Drop Profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            profileSetPtrRestore);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpSet %d %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]);

        /* AUTODOC: Restore the CNM Target Assignment Mode */
        rc = prvTgfPortCnQueueStatusModeEnableSet(prvTgfDevNum, enablePtrRestore,
             targetHwDevPtrRestore, targetPortPtrRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortCnQueueStatusModeEnableGet %d %d %d",
            enablePtrRestore, targetHwDevPtrRestore, targetPortPtrRestore);

        /* AUTODOC: Restore the Fb calculation configuration */
        rc = prvTgfPortCnFbCalcConfigSet(prvTgfDevNum, &fbCalcCfgPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnFbCalcConfigGet");

        /* AUTODOC: Restore the configured QCN trigger values */
        rc = prvTgfPortCnQcnTriggerSet(prvTgfDevNum, index,
            &qcnTriggerPtrRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortCnQcnTriggerSet");

        /* AUTODOC: Restore the Ether Type for QCN frames */
        rc = prvTgfPortCnEtherTypeSet(prvTgfDevNum, etherTypePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnEtherTypeGet %d",
            etherTypePtrRestore);

        /* AUTODOC: Restore cn enable/disable mechanism */
        rc = prvTgfPortCnDbaModeEnableSet(prvTgfDevNum, cnModePtrRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortCnDbaModeEnableGet: %d", cnModePtrRestore);

        /* AUTODOC: Restore configured resource mode for QCN DBA */
        rc = prvTgfPortTxTailDropGlobalParamsSet(prvTgfDevNum, resourceModeRestore,
             globalAvailableBuffersRestore, pool0AvailableBuffersRestore, pool1AvailableBuffersRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxTailDropGlobalParamsGet: %d", resourceModeRestore);

        if(resource == CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E)
        {
            /* AUTODOC: Restore number of global available buffers for DBA QCN */
            rc = prvTgfPortCnDbaAvailableBuffSet(prvTgfDevNum,
                PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaAvailableBuffSet: %d",
                PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
        }
        else
        {
            /* AUTODOC: Restore number of pool 0/1 available buffers for DBA QCN */
            rc = prvTgfPortCnDbaPoolAvailableBuffSet(prvTgfDevNum,
                poolNum, poolAvailableBuffRestore);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortCnDbaPoolAvailableBuffSet: %d",
                PRV_TGF_QCN_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
        }

        /* AUTODOC: Restore VLAN related config  */
        rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = cpssDxChBrgMcMemberAdd(prvTgfDevNum, 0xFFF, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgMcMemberAdd: %d", prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

        /* AUTODOC: Reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS] = portForCpuRestore;
    }
}

/**
* @internal tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic for tgfPortTxSip6QcnDbaQueueResources test.
*
*/
GT_VOID tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate
(
    GT_BOOL ingressPktHdr
)
{
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                                        buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                                        packetActualLength = 0;
    GT_U32                                        ii;
    GT_U8                                         queue;
    GT_U8                                         devNum = prvTgfDevNum;
    GT_HW_DEV_NUM                                 hwDevNum;
    CPSS_DXCH_NET_RX_PARAMS_STC                   dxChPcktParams;
    GT_U32                                        numOfCpuPackets = 0;
    GT_BOOL                                       encapsulatedSDU = GT_FALSE;
    GT_BOOL                                       dstMacCheck = GT_TRUE;
    GT_BOOL                                       payloadCheck = GT_TRUE;
    GT_BOOL                                       expectedSduLengthCheck = 0;
    GT_U16                                        SDULength;
    GT_U16                                        qcnSduOffset;
    GT_U16                                        sduLenOffset;
    GT_U16                                        encapMacDaOffset;
    GT_U16                                        vlanVidStart;
    GT_U16                                        payloadStart;
    GT_U16                                        vlanStart;
    GT_U8                                         pri;
    GT_U8                                         cfi;
    GT_U16                                        vid;
    TGF_ETHER_TYPE                                etherType;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT          alpha;
    GT_U32                                        expectedValue = 0;
    static CPSS_PORT_CN_PROFILE_CONFIG_STC        cnProfileCfgParams;

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    /* AUTODOC: Go over all alphas */
    for (ii = 0; ii < PRV_TGF_COUNT_OF_SIP6_QUEUE_TRANSMITTING_CNS; ii++)
    {
        alpha = prvTgfSip6QcnDbaQueueAlpha[ii];

        PRV_UTF_LOG1_MAC("ALPHA:   %s\n",
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E)       ? "0.0"     :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E)    ? "0.25"    :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E)     ? "0.5"     :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E)       ? "1.0"     :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E)       ? "2.0"     :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E)       ? "4.0"     :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E)   ? "0.125"   :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E)       ? "8.0"     :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E) ? "0.03125" :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E)  ? "0.0625"  :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E)      ? "16.0"    :
            (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E)      ? "32.0"    :
            "BAD PARAMETER");

        /* AUTODOC: Configure qcn profile config including alpha for traffic class 15 and drop profile 7*/
        cpssOsMemSet(&cnProfileCfgParams, 0, sizeof(cnProfileCfgParams));
        cnProfileCfgParams.cnAware = GT_TRUE;
        cnProfileCfgParams.threshold = prvTgfSip6QcnDbaQueueLimits[11];
        cnProfileCfgParams.alpha = alpha;

        st = prvTgfPortCnProfileQueueConfigSet(prvTgfDevNum,
            PRV_TGF_TD_PROFILE_CNS, PRV_TGF_SIP6_TRAFFIC_CLASS_CNS, &cnProfileCfgParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfPortCnProfileQueueConfigSet");

        /* Get expected amount of available buffers */
        st = prvTgfPortTxSip6QcnDbaQueueExpectedAmountOfBuffers(ii,
            &expectedValue);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
            "prvTgfPortTxSip6QcnDbaQueueExpectedAmountOfBuffers");

        /* AUTODOC: Block TxQ (15) for receive port (1).*/
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_SIP6_TRAFFIC_CLASS_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_SIP6_TRAFFIC_CLASS_CNS, GT_FALSE);

        /* clear table */
        st = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

        /* start Rx capture */
        st = tgfTrafficTableRxStartCapture(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

        /* AUTODOC: Transmit 100 frames (with MAC DA 00:01:02:03:AA:01) to
           port 0. */
        st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
            PRV_TGF_QCN_COUNT_OF_FRAMES_DBA_QCN_TO_TRANSMIT_CNS, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
            PRV_TGF_QCN_COUNT_OF_FRAMES_DBA_QCN_TO_TRANSMIT_CNS, 0, NULL);

        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, 1);

        cpssOsTimerWkAfter(1);

        prvTgfPrintPortCountersEth();

        /* AUTODOC: Enable TxQ (15) for receive port (1).*/
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_SIP6_TRAFFIC_CLASS_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_SIP6_TRAFFIC_CLASS_CNS, GT_TRUE);

        prvTgfPrintPortCountersEth();

        /* stop Rx capture */
        st = tgfTrafficTableRxStartCapture(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "tgfTrafficTableRxStartCapture");

        st = extUtilHwDeviceNumberGet(devNum, &hwDevNum);
        if(st != GT_OK)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            return;
        }
        numOfCpuPackets = 0;
        do
        {
            st = tgfTrafficTableRxPcktGet(numOfCpuPackets++, packetBuff, &buffLen, &packetActualLength,
                &devNum, &queue, (GT_VOID *)&dxChPcktParams);
            if ((alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E && numOfCpuPackets < expectedValue) ||
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E && numOfCpuPackets < expectedValue))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktGet");
            }

            if (st == GT_OK)
            {
                PRV_UTF_LOG1_MAC(" QCN Packet [%d] in CPU\n", numOfCpuPackets);
                tgfTrafficTracePacket(packetBuff, buffLen, GT_TRUE);

                UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.commonParams.dsaTagType,
                    CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT, "dsaTagType different than expected which is extended DSA tag - 4 words");

                UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.dsaType,
                    CPSS_DXCH_NET_DSA_CMD_FORWARD_E, "dsaType is different than expected which is FORWARD");

                UTF_VERIFY_EQUAL0_STRING_MAC(hwDevNum,
                    dxChPcktParams.dsaParam.dsaInfo.forward.srcHwDev,
                    "sampled packet dev num different than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    dxChPcktParams.dsaParam.dsaInfo.forward.source.portNum,
                    "sampled packet port num different than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(hwDevNum,
                    dxChPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum,
                    "qcn pkt target dev num different than expected");

                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                    dxChPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum,
                    "qcn pkt target port num different than expected");

                /* QCN pkt SDU length decides if IngressPktHdr is part of QCN. SDU
                 * length is at offset 41-42 in QCN frame which is part of QCN Hdr.
                 * [DA(6)+SA(6)+VLAN(4)+CNEtherType(2)+QCNHdr(24)]
                 */
                qcnSduOffset = TGF_QCN_DA_CNS + TGF_QCN_SA_CNS + TGF_QCN_VLAN_CNS + TGF_QCN_ETHER_TYPE_CNS + TGF_QCN_HDR_CNS;
                sduLenOffset = qcnSduOffset - TGF_QCN_HDR_SDU_LENGTH_CNS;
                encapMacDaOffset = sduLenOffset - TGF_QCN_DA_CNS;
                SDULength = (GT_U16)(packetBuff[sduLenOffset + 1] | ((packetBuff[sduLenOffset] & 0xFF) << 8));
                encapsulatedSDU = (SDULength > 0) ? 1 : 0;
                if (ingressPktHdr)
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(1, encapsulatedSDU,
                        "encapsulatedSDULLength zero and expected is non-zero");

                    /* Encapsulated SDU can be max 86 bytes */
                    if (encapsulatedSDU && SDULength <= 86)
                    {
                        expectedSduLengthCheck = 1;
                    }
                    UTF_VERIFY_EQUAL1_STRING_MAC(1, expectedSduLengthCheck,
                        "encapsulatedSDU greater than expected %d", SDULength);

                    /* Encapsulated MAC DA check */
                    dstMacCheck = cpssOsMemCmp(&(prvTgfPacketL2Part.daMac), &packetBuff[encapMacDaOffset], sizeof(TGF_QCN_DA_CNS));
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, dstMacCheck,
                        "dstMac os sampled pkt different than expected");

                    /* Vlan Check of the SDU [TPID, CFI, UP, VID] */
                    vlanStart = qcnSduOffset;
                    vlanVidStart = qcnSduOffset + TGF_QCN_VLAN_TPID_LENGTH_CNS;

                    etherType = (TGF_ETHER_TYPE) (packetBuff[vlanStart + 1] | (packetBuff[vlanStart] << 8));
                    vid = (GT_U16)(packetBuff[vlanVidStart + 1] | ((packetBuff[vlanVidStart] & 0x0F) << 8));
                    pri = (GT_U8)((packetBuff[vlanVidStart] >> 5) & 0x7);
                    cfi = (GT_U8)((packetBuff[vlanVidStart] >> 4) & 0x1);
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.etherType, etherType,
                        "TPID of sampled pkt different than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.pri, pri,
                        "vlanPri of sampled pkt different than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.cfi, cfi,
                        "vlanCfi of sampled pkt different than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketVlanTagPart.vid, vid,
                        "vlanVid of sampled pkt different than expected");

                    /* Payload check of the SDU, first 20 bytes */
                    payloadStart = qcnSduOffset + TGF_QCN_VLAN_CNS;
                    payloadCheck = cpssOsMemCmp(prvTgfPayloadDataArr,&packetBuff[payloadStart], 20);
                    UTF_VERIFY_EQUAL0_STRING_MAC(0, payloadCheck,
                        "ingressHdrInQCN bytes different than payload sent");
                }
                else
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(0,
                        encapsulatedSDU,
                        "encapsulatedSDULLength non-zero and expected is zero");
                }
            }
        } while (st == GT_OK);

        /* exclude GT_NOT_FOUND from calculation */
        --numOfCpuPackets;

        UTF_VERIFY_EQUAL0_STRING_MAC(expectedValue, numOfCpuPackets,
            "number of QCN packets different than expected");

        st = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");
    }
}

