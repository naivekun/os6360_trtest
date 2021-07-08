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
* @file prvTgfPortTxTailDropDba.c
*
* @brief "DBA Queue Resources Allocation" enhanced UT for Tx Tail Drop APIs
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

#define PRV_TGF_RECV_PORT_IDX_CNS 1
#define PRV_TGF_SEND_PORT_IDX_CNS 0
#define CPSS_TC_RANGE_CNS 8
#define PRV_TGF_TD_PROFILE_CNS CPSS_PORT_TX_DROP_PROFILE_7_E

#define PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS 1000
#define PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS 3
#define PRV_TGF_COUNT_OF_PORT_TRANSMITTING_CNS /*8 - 0.125 and 8 are not supported yet*/ 6
#define PRV_TGF_VLANID_CNS 7
#define PRV_TGF_TRAFFIC_CLASS_CNS 7
#define PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS 1000
#define PRV_TGF_RCVPRT_COS_PROFILE_CNS 1023

/* Convert Tail Drop DBA Alpha to ratio value (GT_FLOAT32) */
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_RATIO_CONVERT_MAC(alpha, ratio) \
    do {                                                                    \
        switch (alpha)                                                      \
        {                                                                   \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                      \
                ratio = (0.0);                                              \
                break;                                                      \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E:                  \
                ratio = (0.125);                                            \
                break;                                                      \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                   \
                ratio = (0.25);                                             \
                break;                                                      \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                    \
                ratio = (0.5);                                              \
                break;                                                      \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                      \
                ratio = (1.0);                                              \
                break;                                                      \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                      \
                ratio = (2.0);                                              \
                break;                                                      \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                      \
                ratio = (4.0);                                              \
                break;                                                      \
            case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E:                      \
                ratio = (8.0);                                              \
                break;                                                      \
            default:                                                        \
                return GT_BAD_PARAM;                                        \
        }                                                                   \
    } while (0)

/* Alpha for dynamic buffers allocation per {TC,DP} */
static CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
            prvTgfTailDropDbaQueueDbaDpAlpha[PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS] = {
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E,      /* DP0 (green) Alpha */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E,      /* DP1 (yellow) Alpha */
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E    /* DP2 (red) Alpha */
};

/* Alpha for dynamic buffers allocation per port */
static CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
                    prvTgfTailDropDbaPortDbaPortAlpha[PRV_TGF_COUNT_OF_PORT_TRANSMITTING_CNS] = {
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
    /*CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E,*/
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E /*,
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E */
};

/* guaranteed buffers limit per Drop precedence */
static GT_U32 prvTgfTailDropDbaQueueDpLimits[PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS] = {
    300,    /* DP0 (green) buffers limit */
    200,    /* DP1 (yellow) buffers limit */
    100     /* DP2 (red) buffers limit */
};

static GT_U32 prvTgfTailDropDbaPortDbaPortMaxBuffLimit[PRV_TGF_COUNT_OF_PORT_TRANSMITTING_CNS] = {
    600,    /* for Alpha ratio 0.0   ( 0.0% of dynamic buffers on this UT) */
/*    550, */    /* for Alpha ratio 0.125 (11.1% of dynamic buffers on this UT) */
    500,    /* for Alpha ratio 0.25  (20.0% of dynamic buffers on this UT) */
    400,    /* for Alpha ratio 0.5   (33.3% of dynamic buffers on this UT) */
    300,    /* for Alpha ratio 1.0   (50.0% of dynamic buffers on this UT) */
    200,    /* for Alpha ratio 2.0   (66.6% of dynamic buffers on this UT) */
    100 /*, */    /* for Alpha ratio 4.0   (80.0% of dynamic buffers on this UT) */
/*    50 */      /* for Alpha ratio 8.0   (88.8% of dynamic buffers on this UT) */
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

/**
* @internal prvTgfPortTxTailDropDbaQueueExpectedAmountOfBuffers function
* @endinternal
*
* @brief Calculate expected amount of available buffers (include dynamic
*        allocation) for per queue resources test.
*
* @retval GT_OK        -- Success
* @retval GT_BAD_PARAM -- Bad alpha value in prvTgfTailDropDbaQueueDbaDpAlpha[dp]
*
* @param[in] dp - Drop precedence
*
* @param[out] expectedValue - expected amount of available buffers
*/
static GT_STATUS prvTgfPortTxTailDropDbaQueueExpectedAmountOfBuffers
(
    IN  CPSS_DP_LEVEL_ENT   dp,
    OUT GT_U32              *expectedValue
)
{
    GT_U32      dynamicFactor, usedBuffers;
    GT_FLOAT32  sumOfAllDestPortsRatio;
    GT_FLOAT32  ratio;

    /* Convert enum Alpha to float */
    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_RATIO_CONVERT_MAC(
            prvTgfTailDropDbaQueueDbaDpAlpha[dp], ratio);

    /* test use only one destination port */
    sumOfAllDestPortsRatio = ratio;

    /* test uses guarantied number of buffers before DBA start to work */
    usedBuffers = prvTgfTailDropDbaQueueDpLimits[dp];


    /*
       Dynamic factor calculated as:

                                                     Alpha
          DynamicFactor = FreeBuffers * ----------------------------
                                        1 + sumOfAllDestPortsRatio

       where sumOfAllDestPortsRatio - sum of Alphas of all destination ports.
       On this test we set Alphas for Tail Drop Profile 0 (default profile)
       to 0 - so we have Alpha0 on all ports except receive. On receive port
       we use Tail Drop Profile 7 with Alphas:

          {PRV_TGF_TRAFFIC_CLASS_CNS, DP0..DP2} = prvTgfTailDropDbaQueueDbaDpAlpha[0..2]
          {All TC exclude PRV_TGF_TRAFFIC_CLASS_CNS, DP0..DP2} = 0

    */
    dynamicFactor = (GT_U32)((PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS - usedBuffers)      /* Free Buffers */
                    * (ratio / (1 + sumOfAllDestPortsRatio)));  /* Alpha / (1 + sumOfAllDestPortsRatio) */

    /* Limit = Guaranteed Buffers + Dynamic Factor */
    *expectedValue = prvTgfTailDropDbaQueueDpLimits[dp] + dynamicFactor;

    return GT_OK;
}

/**
* @internal prvTgfPortTxTailDropDbaPortExpectedAmountOfBuffers function
* @endinternal
*
* @brief Calculate expected amount of available buffers (include dynamic
*        allocation) for per port eresources test.
*
* @retval GT_OK        -- Success
* @retval GT_BAD_PARAM -- Bad alpha value in prvTgfTailDropDbaPortDbaPortAlpha[numOfTransmit]
*
* @param[in] numOfTransmit - number of transmitting
*
* @param[out] expectedValue - expected amount of available buffers
*/
static GT_STATUS prvCpssPxTgfPortTxTailDropDbaPortExpectedAmountOfBuffers
(
    IN  GT_U32  numOfTransmit,
    OUT GT_U32  *expectedValue
)
{
    GT_U32      dynamicFactor, usedBuffers;
    GT_FLOAT32  ratio;

    /* Convert enum Alpha to float */
    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_RATIO_CONVERT_MAC(
            prvTgfTailDropDbaPortDbaPortAlpha[numOfTransmit], ratio);

    /* test uses guarantied number of buffers before DBA start to work */
    usedBuffers = prvTgfTailDropDbaPortDbaPortMaxBuffLimit[numOfTransmit];

    /*
       Dynamic factor calculated as:

                                                   Alpha
          DynamicFactor = FreeBuffers * ----------------------------
                                         1 + sumOfAllDestPortsRatio

       where sumOfAllDestPortsRatio - sum of Alphas of all destination ports.
       On this test we set Alphas for Tail Drop Profile 0 (default profile)
       to 0 - so we have Alpha0 on all ports except receive. On just one
       port (receive) we use Tail Drop Profile 7 with different Alphas (see
       variable prvTgfTailDropDbaPortDbaPortAlpha). So:

          sumOfAllDestPortsRatio = ratio    - because just one port (receive)
                                              will have non-zero Alpha
    */
    dynamicFactor = (GT_U32)((PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS - usedBuffers) /* Free Buffers */
                    * (ratio / (1 + ratio)));              /* Alpha / (1 + sumOfAllDestPortsRatio) */

    /* Limit = Guaranteed Buffers + Dynamic Factor */
    *expectedValue = usedBuffers + dynamicFactor;

    return GT_OK;
}

/**
* @internal tgfPortTxTailDropDbaQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxTailDropDbaQueueResources test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxTailDropDbaQueueResourcesConfigure
(
    GT_BOOL configSet
)
{
    GT_STATUS   rc;
    GT_U8       tc;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  profileParams;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  defProfileParams;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  *profileParamsPtr;
    static CPSS_PORT_TX_DROP_PROFILE_SET_ENT    recvPortProfileIdRestore;
    static GT_BOOL                              dbaEnabledRestore;
    static PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tdropTcProfileParamsRestore[CPSS_TC_RANGE_CNS];
    static CPSS_QOS_ENTRY_STC                   qosRecvPortConfigRestore;
    CPSS_QOS_ENTRY_STC                          qosRecvPortConfig;
    static PRV_TGF_COS_PROFILE_STC              cosProfileRestore;
    static CPSS_QOS_PORT_TRUST_MODE_ENT         trustModeRestore;
    static GT_U32                               dbaAvailableBufferRestore;
    GT_U8                                       vlanTags[2];
    static GT_U32                               profPortMaxBufRestore;
    static GT_U32                               profPortMaxDescRestore;
    static CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT profPortAlphaRestore;

    if(configSet)
    {
        /* AUTODOC: set vlan entry */
        vlanTags[0] = 1;
        vlanTags[1] = 1;
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
            NULL, vlanTags, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_CNS);

        /* AUTODOC: Save receive port (1) to Tail Drop Profile*/
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            &recvPortProfileIdRestore);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpGet %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]
        );

        /* AUTODOC: Set HOL system mode */
        rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet");

        /* AUTODOC: Bind receive port (1) to Tail Drop Profile 7 */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TD_PROFILE_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpSet %d %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TD_PROFILE_CNS
        );

        /* AUTODOC: Save DBA mode enabled state */
        rc = prvTgfPortTxDbaEnableGet(prvTgfDevNum, &dbaEnabledRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaEnableGet %d",
            prvTgfDevNum);

        /* AUTODOC: Enable DBA */
        rc = prvTgfPortTxDbaEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaEnableGet %d %d",
            prvTgfDevNum, GT_TRUE);

        /* AUTODOC: Save limits and alpha for traffic class and profile */
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                    tc,
                    &tdropTcProfileParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");
        }

        /* AUTODOC: Configure limits and alpha for traffic class and profile */
        cpssOsMemSet(&profileParams, 0, sizeof(profileParams));
        profileParams.dp0MaxBuffNum = prvTgfTailDropDbaQueueDpLimits[0];
        profileParams.dp1MaxBuffNum = prvTgfTailDropDbaQueueDpLimits[1];
        profileParams.dp2MaxBuffNum = prvTgfTailDropDbaQueueDpLimits[2];
        profileParams.dp0MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.dp1MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.dp2MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.tcMaxBuffNum   = 0;
        profileParams.tcMaxDescrNum  = 0;
        profileParams.dp0QueueAlpha = prvTgfTailDropDbaQueueDbaDpAlpha[0];
        profileParams.dp1QueueAlpha = prvTgfTailDropDbaQueueDbaDpAlpha[1];
        profileParams.dp2QueueAlpha = prvTgfTailDropDbaQueueDbaDpAlpha[2];

        cpssOsMemSet(&defProfileParams, 0, sizeof(defProfileParams));
        defProfileParams.dp0MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defProfileParams.dp1MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defProfileParams.dp2MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defProfileParams.dp0MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defProfileParams.dp1MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defProfileParams.dp2MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        defProfileParams.tcMaxBuffNum   = 0;
        defProfileParams.tcMaxDescrNum  = 0;
        defProfileParams.dp0QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        defProfileParams.dp1QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        defProfileParams.dp2QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            /* Set Alpha0 for Tail Drop Profile 7 for all TCs exclude TC used on test */
            profileParamsPtr = (tc == PRV_TGF_TRAFFIC_CLASS_CNS)
                        ? &profileParams
                        : &defProfileParams;

            rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc, profileParamsPtr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");
        }

        /* AUTODOC: Save number of available buffers */
        rc = prvTgfPortTxDbaAvailBuffGet(prvTgfDevNum,
            &dbaAvailableBufferRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaAvailBuffGet");

        /* AUTODOC: Set number of available buffers */
        rc = prvTgfPortTxDbaAvailBuffSet(prvTgfDevNum,
            PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaAvailBuffSet: %d",
            PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);

        /* AUTODOC: Save buffer limit, descriptor limit and alpha for profile */
        rc = prvTgfPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
            &profPortAlphaRestore, &profPortMaxBufRestore,
            &profPortMaxDescRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxTailDropProfileGet: %d", PRV_TGF_TD_PROFILE_CNS);

        /* AUTODOC: Set buffer limit, descriptor limit and alpha for profile */
        rc = prvTgfPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
                PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS,
                PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxTailDropProfileSet: %d", PRV_TGF_TD_PROFILE_CNS);

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
    }
    else
    {
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

        /* AUTODOC: Restore buffer limit, descriptor limit and alpha for profile */
        rc = prvTgfPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
            profPortAlphaRestore, profPortMaxBufRestore,
            profPortMaxDescRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxTailDropProfileSet: %d", PRV_TGF_TD_PROFILE_CNS);

        /* AUTODOC: Restore number of available buffers */
        rc = prvTgfPortTxDbaAvailBuffSet(prvTgfDevNum, dbaAvailableBufferRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaAvailBuffSet: %d",
            dbaAvailableBufferRestore);

        /* AUTODOC: Restore limits and alpha for traffic class and profile */
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc,  &tdropTcProfileParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");
        }

        /* AUTODOC: Restore DBA mode enabled state */
        rc = prvTgfPortTxDbaEnableSet(prvTgfDevNum, dbaEnabledRestore);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaEnableGet %d %d",
            prvTgfDevNum, dbaEnabledRestore);

        /* AUTODOC: Restore receive port (1) Tail Drop Profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            recvPortProfileIdRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpSet %d %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            recvPortProfileIdRestore
        );

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
* @internal tgfPortTxTailDropDbaQueueResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxTailDropDbaQueueResources test.
*
*/
GT_VOID tgfPortTxTailDropDbaQueueResourcesTrafficGenerate()
{
    GT_U32 ii;
    GT_U32 expectedValue = 0;
    GT_STATUS st;
    PRV_TGF_COS_PROFILE_STC cosProfile;
    CPSS_PORT_MAC_COUNTER_SET_STC portCounters;

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    /* AUTODOC: Go over all Drop Precedences (green/yellow/red) */
    for (ii = 0; ii < PRV_TGF_COUNT_OF_QUEUE_TRANSMITTING_CNS; ii++)
    {
        /* AUTODOC: Get expected amount of available buffers (expectedValue). */
        st = prvTgfPortTxTailDropDbaQueueExpectedAmountOfBuffers(ii,
            &expectedValue);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvTgfPortTxTailDropDbaQueueExpectedAmountOfBuffers", ii);

        /* AUTODOC: Configure COS profile 1023 with Drop Precedence and
           Traffic Class (7) of ingress frames. */
        cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));
        cosProfile.trafficClass = PRV_TGF_TRAFFIC_CLASS_CNS;
        switch(ii)
        {
            case 0:
                cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
                break;
            case 1:
                cosProfile.dropPrecedence = CPSS_DP_YELLOW_E;
                break;
            case 2:
                cosProfile.dropPrecedence = CPSS_DP_RED_E;
                break;
        }
        st = prvTgfCosProfileEntrySet(PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfile);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfCosProfileEntrySet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);

        /* AUTODOC: Block TxQ (7) for receive port (1).*/
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);

        /* AUTODOC: Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to
           port 0. */
        st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
            PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
            PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, 1);

        /* AUTDOC: Release TxQ (7) for receive port (1) */
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);

        cpssOsTimerWkAfter(1);

        /* AUTODOC: Check MAC counters. */
        st = prvTgfReadPortCountersEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], GT_FALSE,
            &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfReadPortCountersEth");

        if ((portCounters.goodPktsSent.l[0] < (expectedValue - 10)) ||
            (portCounters.goodPktsSent.l[0] > (expectedValue + 10)))
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(expectedValue, portCounters.goodPktsSent.l[0],
                    "Got other counter value than expected:\n"
                    "   Tx counter:      %d\n"
                    "   Expected range:  [%d..%d]\n",
                    portCounters.goodPktsSent.l[0],
                    expectedValue - 10, expectedValue + 10);
        }
        else
        {
            PRV_UTF_LOG3_MAC("Got Tx counter value as expected:\n"
                             "   Tx counter:      %d\n"
                             "   Expected range:  [%d..%d]\n",
                             portCounters.goodPktsSent.l[0],
                             expectedValue - 10, expectedValue + 10);
        }
    }
}

/**
* @internal tgfPortTxTailDropDbaPortResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxTailDropDbaPortResources test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxTailDropDbaPortResourcesConfigure
(
    GT_BOOL configSet
)
{
    GT_STATUS   rc;
    GT_U8       tc;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS  profileParams;
    static CPSS_PORT_TX_DROP_PROFILE_SET_ENT    recvPortProfileIdRestore;
    static GT_BOOL                              dbaEnabledRestore;
    static PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS tdropTcProfileParamsRestore[CPSS_TC_RANGE_CNS];
    static CPSS_QOS_ENTRY_STC                   qosRecvPortConfigRestore;
    CPSS_QOS_ENTRY_STC                          qosRecvPortConfig;
    static PRV_TGF_COS_PROFILE_STC              cosProfileRestore;
    PRV_TGF_COS_PROFILE_STC                     cosProfile;
    static CPSS_QOS_PORT_TRUST_MODE_ENT         trustModeRestore;
    static GT_U32                               dbaAvailableBufferRestore;
    GT_U8                                       vlanTags[2];
    static GT_U32                               profPortMaxBufRestore;
    static GT_U32                               profPortMaxDescRestore;
    static CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT profPortAlphaRestore;

    if(configSet)
    {
        /* AUTODOC: set vlan entry */
        vlanTags[0] = 1;
        vlanTags[1] = 1;
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
            NULL, vlanTags, 2);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_CNS);

        /* AUTODOC: Set HOL system mode */
        rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet");

        /* AUTODOC: Save receive port (1) to Tail Drop Profile*/
        rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            &recvPortProfileIdRestore);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpGet %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]
        );

        /* AUTODOC: Bind receive port (1) to Tail Drop Profile 7 */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TD_PROFILE_CNS);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpSet %d %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TD_PROFILE_CNS
        );

        /* AUTODOC: Save DBA mode enabled state */
        rc = prvTgfPortTxDbaEnableGet(prvTgfDevNum, &dbaEnabledRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaEnableGet %d",
            prvTgfDevNum);

        /* AUTODOC: Enable DBA */
        rc = prvTgfPortTxDbaEnableSet(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaEnableGet %d %d",
            prvTgfDevNum, GT_TRUE);

        /* AUTODOC: Save limits and alpha for traffic class and profile */
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
                    tc,
                    &tdropTcProfileParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcGet");
        }

        /* AUTODOC: Configure limits and alpha for traffic class and profile */
        cpssOsMemSet(&profileParams, 0, sizeof(profileParams));
        profileParams.dp0MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.dp1MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.dp2MaxBuffNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.dp0MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.dp1MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.dp2MaxDescrNum = PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS;
        profileParams.tcMaxBuffNum   = 0;
        profileParams.tcMaxDescrNum  = 0;
        profileParams.dp0QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        profileParams.dp1QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        profileParams.dp2QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;

        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc, &profileParams);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");
        }

        /* AUTODOC: Save number of available buffers */
        rc = prvTgfPortTxDbaAvailBuffGet(prvTgfDevNum,
            &dbaAvailableBufferRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaAvailBuffGet");

        /* AUTODOC: Set number of available buffers */
        rc = prvTgfPortTxDbaAvailBuffSet(prvTgfDevNum,
            PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaAvailBuffSet: %d",
            PRV_TGF_AMOUNT_OF_BUFFERS_FOR_DBA_CNS);

        /* AUTODOC: Save buffer limit, descriptor limit and alpha for profile */
        rc = prvTgfPortTxTailDropProfileGet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
            &profPortAlphaRestore, &profPortMaxBufRestore,
            &profPortMaxDescRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxTailDropProfileGet: %d", PRV_TGF_TD_PROFILE_CNS);

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

        /* AUTODOC: Set CoS entry */
        cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));
        cosProfile.trafficClass = PRV_TGF_TRAFFIC_CLASS_CNS;
        rc = prvTgfCosProfileEntrySet(PRV_TGF_RCVPRT_COS_PROFILE_CNS,
            &cosProfile);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet %d",
            PRV_TGF_RCVPRT_COS_PROFILE_CNS);
    }
    else
    {
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

        /* AUTODOC: Restore number of available buffers */
        rc = prvTgfPortTxDbaAvailBuffSet(prvTgfDevNum, dbaAvailableBufferRestore);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaAvailBuffSet: %d",
            dbaAvailableBufferRestore);

        /* AUTODOC: Restore limits and alpha for traffic class and profile */
        for (tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum,
                    PRV_TGF_TD_PROFILE_CNS, tc,  &tdropTcProfileParamsRestore[tc]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssPxPortTxTailDropProfileTcSet");
        }

        /* AUTODOC: Restore DBA mode enabled state */
        rc = prvTgfPortTxDbaEnableSet(prvTgfDevNum, dbaEnabledRestore);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxDbaEnableGet %d %d",
            prvTgfDevNum, dbaEnabledRestore);

        /* AUTODOC: Restore receive port (1) Tail Drop Profile */
        rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            recvPortProfileIdRestore);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
            "prvTgfPortTxBindPortToDpSet %d %d %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            recvPortProfileIdRestore
        );

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
* @internal tgfPortTxTailDropDbaPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxTailDropDbaPortResources test.
*
*/
GT_VOID tgfPortTxTailDropDbaPortResourcesTrafficGenerate()
{
    GT_U32 ii;
    GT_U32 expectedValue = 0;
    GT_STATUS st;
    CPSS_PORT_MAC_COUNTER_SET_STC portCounters;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alpha;

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    /* AUTODOC: Go over all alphas */
    for (ii = 0; ii < PRV_TGF_COUNT_OF_PORT_TRANSMITTING_CNS; ii++)
    {
        alpha = prvTgfTailDropDbaPortDbaPortAlpha[ii];

        PRV_UTF_LOG1_MAC("ALPHA:   %s\n",
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E)     ? "0.0"   :
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E) ? "0.125" :
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E)  ? "0.25"  :
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E)   ? "0.5"   :
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E)     ? "1.0"   :
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E)     ? "2.0"   :
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E)     ? "4.0"   :
                (alpha == CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E)     ? "8.0"   :
                "BAD PARAMETER");

        /* AUTODOC: Get expected amount of available buffers (expectedValue). */
        st = prvCpssPxTgfPortTxTailDropDbaPortExpectedAmountOfBuffers(ii,
            &expectedValue);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvTgfPortTxTailDropDbaQueueExpectedAmountOfBuffers", ii);

        /* set portAlpha for receive port */
        st = prvTgfPortTxTailDropProfileSet(prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
            prvTgfTailDropDbaPortDbaPortAlpha[ii],         /* portAlpha        */
            prvTgfTailDropDbaPortDbaPortMaxBuffLimit[ii],  /* portMaxBuffLimit */
            PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS);      /* portMaxDescLimit */
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxTailDropProfileSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TD_PROFILE_CNS,
            prvTgfTailDropDbaPortDbaPortAlpha[ii], prvTgfTailDropDbaPortDbaPortMaxBuffLimit[ii]);

        /* AUTODOC: Block TxQ (7) for receive port (1).*/
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_FALSE);

        /* AUTODOC: Transmit 1000 frames (with MAC DA 00:01:02:03:AA:01) to
           port 0. */
        st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
            PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
            PRV_TGF_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, 1);

        /* AUTDOC: Release TxQ (7) for receive port (1) */
        st = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfPortTxQueueTxEnableSet %d %d %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            PRV_TGF_TRAFFIC_CLASS_CNS, GT_TRUE);

        cpssOsTimerWkAfter(1);

        /* AUTODOC: Check MAC counters. */
        st = prvTgfReadPortCountersEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], GT_FALSE,
            &portCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfReadPortCountersEth");

        if ((portCounters.goodPktsSent.l[0] < (expectedValue - 10)) ||
            (portCounters.goodPktsSent.l[0] > (expectedValue + 10)))
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(expectedValue, portCounters.goodPktsSent.l[0],
                    "Got other counter value than expected:\n"
                    "   Tx counter:      %d\n"
                    "   Expected range:  [%d..%d]\n",
                    portCounters.goodPktsSent.l[0],
                    expectedValue - 10, expectedValue + 10);
        }
        else
        {
            PRV_UTF_LOG3_MAC("Got Tx counter value as expected:\n"
                             "   Tx counter:      %d\n"
                             "   Expected range:  [%d..%d]\n",
                             portCounters.goodPktsSent.l[0],
                             expectedValue - 10, expectedValue + 10);
        }
    }
}
