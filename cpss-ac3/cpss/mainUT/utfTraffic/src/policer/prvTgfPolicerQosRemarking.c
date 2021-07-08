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
* @file prvTgfPolicerQosRemarking.c
*
* @brief Specific Policer features testing
*
* @version   22
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <policer/prvTgfPolicerQosRemarking.h>

#include <common/tgfCommon.h>
#include <common/tgfIpGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfPortGen.h>


/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            3

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_NUM_CNS     0

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_NUM_CNS  1

/* count of packets to send */
#define PRV_TGF_SEND_COUNT_CNS        1

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* PCL Rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* QoS Profile index */
static GT_U32   prvTgfQosProfileIndex = 5;

/* Egress Counters Set number */
static GT_U8   prvTgfEgressCntrSetNum = 0;

/* Qos Profile index for Ingress Remark for Yellow packets */
static GT_U32   prvTgfYellowQosTableRemarkIndex = 10;

/* Qos Profile index for Ingress Remark for Red packets */
static GT_U32   prvTgfRedQosTableRemarkIndex = 20;

/* Qos Profile index for Ingress Remark for Green packets */
static GT_U32   prvTgfGreenQosTableRemarkIndex = 15;

/* Policy Counters for check */
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrGreen;
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrRed;
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrYellow;
static PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntrDrop;


/************************ common parts of packets **************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x22, 0x22, 0x22, 0x22},               /* dstMac */
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x11}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    5, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/************************* packet 1 (IPv4 packet) **************************/


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};


/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of single tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)


/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};


/****************** metering configurations ********************/

#define BURST_RATIO 65535
#define CIR_RATIO   131071

typedef struct ratePair {
    GT_U32 cir;
    GT_U32 cbs;
}ratePair;


static ratePair prvTgfCirAndCbs_100G_Array[16] =  {{BURST_RATIO, BURST_RATIO},  /*rate type 0 MAX cir, MAX cbs */
                                                    {55000, BURST_RATIO},            /*rate type 0*/
                                                    {45000, (BURST_RATIO - 10000)},  /*rate type 0*/
                                                    {35000, (BURST_RATIO - 20000)},  /*rate type 0*/

                                                    {CIR_RATIO*10, BURST_RATIO*8},                    /*rate type 1 max cir, max cbs*/
                                                    {(CIR_RATIO - 30000)*10, BURST_RATIO*8},          /*rate type 1, max cbs*/
                                                    {(CIR_RATIO - 50000)*10,(BURST_RATIO - 10000)*8}, /*rate type 1*/
                                                    {(CIR_RATIO - 50000)*10,(BURST_RATIO - 20000)*8}, /*rate type 1*/

                                                    {CIR_RATIO*100, BURST_RATIO*64},                    /*rate type 2 max cir, max cbs*/
                                                    {(CIR_RATIO - 30000)*100, BURST_RATIO*64},          /*rate type 2, max cbs*/
                                                    {(CIR_RATIO - 50000)*100,(BURST_RATIO - 10000)*64}, /*rate type 2*/
                                                    {(CIR_RATIO - 50000)*100,(BURST_RATIO - 20000)*64}, /*rate type 2*/

                                                    /*{CIR_RATIO*1000, BURST_RATIO*512}, 131G*/          /*rate type 3 max cir, max cbs*/
                                                    {(CIR_RATIO - 30000)*1000, BURST_RATIO*512},         /*rate type 3, max cbs*/
                                                    {(CIR_RATIO - 50000)*1000,(BURST_RATIO - 10000)*512},/*rate type 3*/
                                                    {(CIR_RATIO - 50000)*1000,(BURST_RATIO - 20000)*512}, /*rate type 3*/

                                                    {0xffffffff,0xffffffff} /*end*/
                                                   };

static ratePair prvTgfCirAndCbs_10G_Array[12] =   {{BURST_RATIO, BURST_RATIO},  /*rate type 0 MAX cir, MAX cbs */
                                                    {55000, BURST_RATIO},            /*rate type 0*/
                                                    {45000, (BURST_RATIO - 10000)},  /*rate type 0*/
                                                    {35000, (BURST_RATIO - 20000)},  /*rate type 0*/

                                                    {CIR_RATIO*10, BURST_RATIO*8},                    /*rate type 1 max cir, max cbs*/
                                                    {(CIR_RATIO - 30000)*10, BURST_RATIO*8},          /*rate type 1, max cbs*/
                                                    {(CIR_RATIO - 50000)*10,(BURST_RATIO - 10000)*8}, /*rate type 1*/
                                                    {(CIR_RATIO - 50000)*10,(BURST_RATIO - 20000)*8}, /*rate type 1*/

                                                    /*{CIR_RATIO*100, BURST_RATIO*64}, 13G*/            /*rate type 2 max cir, max cbs*/
                                                    {(CIR_RATIO - 30000)*100, BURST_RATIO*64},          /*rate type 2, max cbs*/
                                                    {(CIR_RATIO - 50000)*100,(BURST_RATIO - 10000)*64}, /*rate type 2*/
                                                    {(CIR_RATIO - 50000)*100,(BURST_RATIO - 20000)*64}, /*rate type 2*/

                                                    {0xffffffff,0xffffffff} /*end*/
                                                };

static ratePair prvTgfCirAndCbs_100M_Array[5] =   {{BURST_RATIO, BURST_RATIO},  /*rate type 0 MAX cir, MAX cbs */
                                                  {55000, BURST_RATIO},            /*rate type 0*/
                                                  {45000, (BURST_RATIO - 10000)},  /*rate type 0*/
                                                  {35000, (BURST_RATIO - 20000)},  /*rate type 0*/

                                                  {0xffffffff,0xffffffff} /*end*/
                                                };


/**********************/
/* Restore parameters */
/**********************/

/* save for restore stage metering enable */
static GT_BOOL saveStageMeterEnable[PRV_TGF_POLICER_STAGE_NUM];

/* save for restore port metering enable */
static GT_BOOL savePortMeterEnable[PRV_TGF_POLICER_STAGE_NUM];

/* save for restore meter mode: port or flow */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT
                    saveMeterMode[PRV_TGF_POLICER_STAGE_NUM];

/*save for restore metering entry*/
static PRV_TGF_POLICER_ENTRY_STC saveMeteringEntry;

/*save for restore clear on read*/
static GT_U32 saveClearOnReadMacCounters;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPolicerTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*/
static GT_VOID prvTgfPolicerTestPacketSend
(
    IN  TGF_PACKET_STC *packetInfoPtr
)
{
    GT_STATUS                   rc         = GT_OK;
    GT_U32                      portsCount = prvTgfPortsNum;
    GT_U32                      portIter   = 0;
    CPSS_INTERFACE_INFO_STC     portInterface;


    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS];


    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);


    /* send Packet from port port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS]);

    cpssOsTimerWkAfter(10);


    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

}


/**
* @internal prvTgfPolicerCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS rc = GT_OK;

    cpssOsMemSet(&prvTgfMngCntrGreen, 0, sizeof(prvTgfMngCntrGreen));
    cpssOsMemSet(&prvTgfMngCntrRed, 0, sizeof(prvTgfMngCntrRed));
    cpssOsMemSet(&prvTgfMngCntrYellow, 0, sizeof(prvTgfMngCntrYellow));
    cpssOsMemSet(&prvTgfMngCntrDrop, 0, sizeof(prvTgfMngCntrDrop));

    /* AUTODOC: reset management counters for GREEN packets */
    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    /* AUTODOC: reset management counters for YELLOW packets */
    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    /* AUTODOC: reset management counters for RED packets */
    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    /* AUTODOC: reset management counters for DROPPED packets */
    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfPolicerQosRemarkingPclConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPolicerQosRemarkingPclConfigurationSet
(
    IN CPSS_PCL_DIRECTION_ENT           pclDirection,
    IN GT_U32                            portNum
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         patt;
    PRV_TGF_PCL_ACTION_STC              action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    nonIpKey;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ipv4Key;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ipv6Key;

    /* clear mask, pattern and action */
    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    prvTgfPclRuleIndex = 1;
    /* init PCL Engine for send/receive port */
    if (pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

        /* difine mask, pattern and action */
        cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));

        cpssOsMemCpy(patt.ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac,
                sizeof(prvTgfPacketL2Part.daMac));

        nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        ipv4Key = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        ipv6Key = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    }
    else
    {
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
        action.egressPolicy          = GT_TRUE;

        /* difine mask, pattern and action */
        cpssOsMemSet(mask.ruleEgrStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));

        cpssOsMemCpy(patt.ruleEgrStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac,
                sizeof(prvTgfPacketL2Part.daMac));

        /* use PCL ID to avoid problems with EPCL based WAs */
        mask.ruleEgrStdNotIp.common.pclId = 0x3FF;
        patt.ruleEgrStdNotIp.common.pclId = (GT_U16) PRV_TGF_PCL_DEFAULT_ID_MAC(pclDirection, CPSS_PCL_LOOKUP_0_E, portNum);

        nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
        ipv4Key = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
        ipv6Key = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

        prvTgfPclRuleIndex = prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(prvTgfPclRuleIndex);
    }

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   lookup_0 */
    /* AUTODOC:   nonIpKey STD_NOT_IP */
    /* AUTODOC:   ipv4Key STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        portNum,
        pclDirection,
        CPSS_PCL_LOOKUP_0_E,
        nonIpKey, ipv4Key, ipv6Key);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);


    /* set action */
    action.pktCmd                = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable = GT_TRUE;
    action.policer.policerId     = prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS];

    /* AUTODOC: set PCL rule 1: */
    /* AUTODOC:   cmd=FORWARD, policerId=0 */
    /* AUTODOC:   DA=00:00:22:22:22:22 */
    rc = prvTgfPclRuleSet(
        ruleFormat,
        prvTgfPclRuleIndex, &mask, &patt, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d", prvTgfDevNum);
};


/**
* @internal prvTgfPolicerQosRemarkingConfigurationExtSet function
* @endinternal
*
* @brief   Set configuration.
*/
static GT_VOID prvTgfPolicerQosRemarkingConfigurationExtSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           policerStage,
    IN PRV_TGF_POLICER_STAGE_METER_MODE_ENT     stageMeterMode,
    IN PRV_TGF_POLICER_NON_CONFORM_CMD_ENT      pcktCmd,
    IN PRV_TGF_POLICER_METER_TB_PARAMS_UNT      *tbParamsPtr,
    IN PRV_TGF_COS_PROFILE_STC                  *yellowCosProfilePtr,
    IN PRV_TGF_COS_PROFILE_STC                  *redCosProfilePtr,
    IN PRV_TGF_POLICER_REMARK_MODE_ENT          egressRemarkMode,
    IN PRV_TGF_POLICER_L2_REMARK_MODEL_ENT      egressRemarkModel,
    IN PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT    egressRemarkTableType,
    IN GT_U32                                   egressRemarkParamValue,
    IN CPSS_DP_LEVEL_ENT                        egressConfLevel,
    IN PRV_TGF_POLICER_QOS_PARAM_STC            *egressQosParamPtr,
    IN GT_BOOL                                  greenRemarkingSupport,
    IN PRV_TGF_COS_PROFILE_STC                  *greenCosProfilePtr
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_POLICER_ENTRY_STC           metteringEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U32                              metteringEntryIndex;
    GT_BOOL                             isMeterModePortEnable;
    CPSS_QOS_ENTRY_STC                  portQosCfg;
    GT_BOOL                 useQosProfile = GT_TRUE;
    GT_BOOL                 usePortMode = GT_TRUE;
    PRV_TGF_COS_PROFILE_STC             *cosProfilePtr;

    cosProfilePtr =
        (greenRemarkingSupport == GT_FALSE)
        ? yellowCosProfilePtr : greenCosProfilePtr;

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        usePortMode = GT_FALSE;
    }

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        useQosProfile = GT_FALSE;
        rc = prvTgfPolicerInit();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerInit: %d", policerStage);
    }


    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);


    /* AUTODOC: enable metering */
    rc = prvTgfPolicerMeteringEnableSet(policerStage, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set metered packet size mode to L2_INCLUDE */
    rc = prvTgfPolicerPacketSizeModeSet(policerStage,
            CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerPacketSizeModeSet: %d", prvTgfDevNum);

    /* AUTODOC: set packet based meter resolution algorithm */
    rc = prvTgfPolicerMeterResolutionSet(policerStage,
            PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeterResolutionSet: %d", prvTgfDevNum);

    if (usePortMode == GT_TRUE)
    {
        /* set Policer Global stage mode Port or Flow */
        rc = prvTgfPolicerStageMeterModeSet(policerStage, stageMeterMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: disable policer counting */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, policerStage,
                                      PRV_TGF_POLICER_COUNTING_DISABLE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

    /* select Policer Global stage mode */
    isMeterModePortEnable = (stageMeterMode == PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* c) enable metering per SEND Port */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, policerStage,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS], isMeterModePortEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortMeteringEnableSet: %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS]);

    /* DX non-eArch ingress use CL for management counters.
       eArch devices and Puma use DP or CL according to settings.
       Ingress test checks management counters for CL.
       Egress test checks management counters for DP. */
    if (((useQosProfile == GT_TRUE) && (!UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))) ||
        (policerStage == PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        /* AUTODOC: set counter color mode to Drop Precedence */
        rc = prvTgfPolicerCountingColorModeSet(policerStage, PRV_TGF_POLICER_COLOR_COUNT_DP_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingColorModeSet: %d", prvTgfDevNum);
    }

    /* h) create metering entry with index 0 - it means that port is 0 */
    metteringEntryIndex               = prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS];

    cpssOsMemSet(&metteringEntry, 0, sizeof(metteringEntry));
    metteringEntry.countingEntryIndex = metteringEntryIndex;
    metteringEntry.mngCounterSet      = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    metteringEntry.meterColorMode     = CPSS_POLICER_COLOR_BLIND_E;
    metteringEntry.meterMode          = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;
    metteringEntry.tbParams.srTcmParams.cir = tbParamsPtr->srTcmParams.cir;
    metteringEntry.tbParams.srTcmParams.cbs = tbParamsPtr->srTcmParams.cbs;
    metteringEntry.tbParams.srTcmParams.ebs = tbParamsPtr->srTcmParams.ebs;
    metteringEntry.modifyUp           = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    metteringEntry.modifyDscp         = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    metteringEntry.modifyDp           = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    metteringEntry.modifyTc           = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    metteringEntry.yellowPcktCmd      = pcktCmd;
    metteringEntry.redPcktCmd         = pcktCmd;
    metteringEntry.qosProfile         = prvTgfQosProfileIndex;
    metteringEntry.remarkMode         = egressRemarkMode;

    if (greenRemarkingSupport == GT_TRUE)
    {
        metteringEntry.greenPcktCmd = pcktCmd;
    }

    if (policerStage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* the ingress test uses 'port mode' and the index to the metering table should be 'local port'.*/
        metteringEntryIndex = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(prvTgfDevNum,metteringEntryIndex);
    }

    rc = prvTgfPolicerEntrySet(policerStage,
                               metteringEntryIndex,
                               &metteringEntry, &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerEntrySet: %d %d", prvTgfDevNum, metteringEntryIndex);


    if (policerStage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Ingress Remarking Configurations*/

        if (pcktCmd == PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E)
        {
            rc = prvTgfCosProfileEntrySet((GT_U8)prvTgfQosProfileIndex, yellowCosProfilePtr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);
        }

        /* Make nessasary configurations for "Remark by Table"*/
        if (pcktCmd == PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E)
        {
            if (useQosProfile == GT_TRUE)
            {
                rc = prvTgfCosProfileEntrySet((GT_U8)prvTgfYellowQosTableRemarkIndex,
                                              yellowCosProfilePtr);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);

                rc = prvTgfCosProfileEntrySet((GT_U8)prvTgfRedQosTableRemarkIndex,
                                              redCosProfilePtr);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);

                if (greenRemarkingSupport == GT_TRUE)
                {
                    rc = prvTgfCosProfileEntrySet(
                        (GT_U8)prvTgfGreenQosTableRemarkIndex, greenCosProfilePtr);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);
                }

                /* g) create Relative Qos Remarking Entry with index 5 */
                rc = prvTgfPolicerQosRemarkingEntrySet(
                    policerStage, prvTgfQosProfileIndex,
                    prvTgfGreenQosTableRemarkIndex,
                    prvTgfYellowQosTableRemarkIndex,
                    prvTgfRedQosTableRemarkIndex);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosRemarkingEntrySet: %d", prvTgfDevNum);
            }
            else
            {
                PRV_TGF_POLICER_QOS_PARAM_STC            qosParam;
                qosParam.dp   = yellowCosProfilePtr->dropPrecedence;
                qosParam.dscp = yellowCosProfilePtr->dscp;
                qosParam.exp  = yellowCosProfilePtr->exp;
                qosParam.tc   = yellowCosProfilePtr->trafficClass;
                qosParam.up   = yellowCosProfilePtr->userPriority;

                rc = prvTgfPolicerQosParamsRemarkEntrySet(policerStage, 0 /* use default profile#0*/,
                                                          PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E /* use L2 remarking */,
                                                          0 /* default TC 0*/,
                                                          CPSS_DP_YELLOW_E,
                                                          &qosParam);
            }
            /* Configure Port QoS attributes */
            portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
            portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            portQosCfg.qosProfileId     = prvTgfQosProfileIndex;

            rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS],
                                           &portQosCfg);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d", prvTgfDevNum);

            /* Configure Port to No_Trust */
            rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS],
                                              CPSS_QOS_PORT_NO_TRUST_E);


            /* Configure Egress Counters set0: Port, TC and DP */
            rc = prvTgfPortEgressCntrModeSet(prvTgfEgressCntrSetNum,
                                             CPSS_EGRESS_CNT_PORT_E | CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_DP_E,
                                             prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                             0,
                                             (GT_U8)cosProfilePtr->trafficClass,
                                             cosProfilePtr->dropPrecedence);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", prvTgfDevNum);
        }
    }
    else
    {
        /* Egress Remarking Configurations */

        /* Configure remark model */
        rc = prvTgfPolicerL2RemarkModelSet(policerStage, egressRemarkModel);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerL2RemarkModelSet %d", prvTgfDevNum);


        rc = prvTgfPolicerEgressQosRemarkingEntrySet(egressRemarkTableType,
                                                     egressRemarkParamValue,
                                                     egressConfLevel ,
                                                     egressQosParamPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerEgressQosRemarkingEntrySet: %d", prvTgfDevNum);
    }

    /* reset the value of specified Management and Policy Counters */
    prvTgfPolicerCountersReset(policerStage);
}

/**
* @internal prvTgfPolicerQosRemarkingConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
static GT_VOID prvTgfPolicerQosRemarkingConfigurationSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT           policerStage,
    IN PRV_TGF_POLICER_STAGE_METER_MODE_ENT     stageMeterMode,
    IN PRV_TGF_POLICER_NON_CONFORM_CMD_ENT      pcktCmd,
    IN PRV_TGF_POLICER_METER_TB_PARAMS_UNT      *tbParamsPtr,
    IN PRV_TGF_COS_PROFILE_STC                  *yellowCosProfilePtr,
    IN PRV_TGF_COS_PROFILE_STC                  *redCosProfilePtr,
    IN PRV_TGF_POLICER_REMARK_MODE_ENT          egressRemarkMode,
    IN PRV_TGF_POLICER_L2_REMARK_MODEL_ENT      egressRemarkModel,
    IN PRV_TGF_POLICER_REMARK_TABLE_TYPE_ENT    egressRemarkTableType,
    IN GT_U32                                   egressRemarkParamValue,
    IN CPSS_DP_LEVEL_ENT                        egressConfLevel,
    IN PRV_TGF_POLICER_QOS_PARAM_STC            *egressQosParamPtr
)
{
    prvTgfPolicerQosRemarkingConfigurationExtSet(
        policerStage,
        stageMeterMode,
        pcktCmd,
        tbParamsPtr,
        yellowCosProfilePtr,
        redCosProfilePtr,
        egressRemarkMode,
        egressRemarkModel,
        egressRemarkTableType,
        egressRemarkParamValue,
        egressConfLevel,
        egressQosParamPtr,
        GT_FALSE /*greenRemarkingSupport*/,
        NULL     /*greenCosProfilePtr*/);
}

/**
* @internal prvTgfPolicerCountersPrint function
* @endinternal
*
* @brief   Print the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerCountersPrint
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS rc = GT_OK;

    /* get and print Management Counters GREEN */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrGreen.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrGreen.duMngCntr.l[0] = %d\n", prvTgfMngCntrGreen.duMngCntr.l[0]);
    if (prvTgfMngCntrGreen.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrGreen.duMngCn tr.l[1] = %d\n", prvTgfMngCntrGreen.duMngCntr.l[1]);
    if (prvTgfMngCntrGreen.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrGreen.packetMngCntr = %d\n", prvTgfMngCntrGreen.packetMngCntr);

    /* get and print Management Counters YELLOW */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrYellow.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrYellow.duMngCntr.l[0] = %d\n", prvTgfMngCntrYellow.duMngCntr.l[0]);
    if (prvTgfMngCntrYellow.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrYellow.duMngCntr.l[1] = %d\n", prvTgfMngCntrYellow.duMngCntr.l[1]);
    if (prvTgfMngCntrYellow.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrYellow.packetMngCntr = %d\n", prvTgfMngCntrYellow.packetMngCntr);

    /* get and print Management Counters RED */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrRed.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrRed.duMngCntr.l[0] = %d\n", prvTgfMngCntrRed.duMngCntr.l[0]);
    if (prvTgfMngCntrRed.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrRed.duMngCntr.l[1] = %d\n", prvTgfMngCntrRed.duMngCntr.l[1]);
    if (prvTgfMngCntrRed.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrRed.packetMngCntr = %d\n", prvTgfMngCntrRed.packetMngCntr);

    /* get and print Management Counters DROP */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
        PRV_TGF_POLICER_MNG_CNTR_SET0_E, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersGet: %d", prvTgfDevNum);

    if (prvTgfMngCntrDrop.duMngCntr.l[0] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrDrop.duMngCntr.l[0] = %d\n", prvTgfMngCntrDrop.duMngCntr.l[0]);
    if (prvTgfMngCntrDrop.duMngCntr.l[1] != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrDrop.duMngCntr.l[1] = %d\n", prvTgfMngCntrDrop.duMngCntr.l[1]);
    if (prvTgfMngCntrDrop.packetMngCntr != 0)
        PRV_UTF_LOG1_MAC("prvTgfMngCntrDrop.packetMngCntr = %d\n", prvTgfMngCntrDrop.packetMngCntr);

    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfPolicerManagementCountersGreenCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Green
*/
static GT_VOID prvTgfPolicerManagementCountersGreenCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS rc = GT_OK;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                                            &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrGreen.duMngCntr.l[0],
        "prvTgfMngCntrGreen.duMngCntr.l[0] = %d\n",
        prvTgfMngCntrGreen.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrGreen.duMngCntr.l[1],
        "prvTgfMngCntrGreen.duMngCntr.l[1] = %d\n",
        prvTgfMngCntrGreen.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrGreen.packetMngCntr,
        "prvTgfMngCntrGreen.packetMngCntr = %d\n",
        prvTgfMngCntrGreen.packetMngCntr);
}

/**
* @internal prvTgfPolicerManagementCountersYellowCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Yellow
*/
static GT_VOID prvTgfPolicerManagementCountersYellowCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS                        rc = GT_OK;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                                            &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrYellow.duMngCntr.l[0],
        "prvTgfMngCntrYellow.duMngCntr.l[0] = %d\n",
        prvTgfMngCntrYellow.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrYellow.duMngCntr.l[1],
        "prvTgfMngCntrYellow.duMngCntr.l[1] = %d\n",
        prvTgfMngCntrYellow.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrYellow.packetMngCntr,
        "prvTgfMngCntrYellow.packetMngCntr = %d\n",
        prvTgfMngCntrYellow.packetMngCntr);
}

/**
* @internal prvTgfPolicerManagementCountersRedCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Red
*/
static GT_VOID prvTgfPolicerManagementCountersRedCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS                        rc = GT_OK;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_RED_E,
                                            &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrRed.duMngCntr.l[0],
        "prvTgfMngCntrRed.duMngCntr.l[0] = %d\n",
        prvTgfMngCntrRed.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrRed.duMngCntr.l[1],
        "prvTgfMngCntrRed.duMngCntr.l[1] = %d\n",
        prvTgfMngCntrRed.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrRed.packetMngCntr,
        "prvTgfMngCntrRed.packetMngCntr = %d\n",
        prvTgfMngCntrRed.packetMngCntr);
}
/**
* @internal prvTgfPolicerManagementCountersDropCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters Drop
*/
static GT_VOID prvTgfPolicerManagementCountersDropCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS                        rc = GT_OK;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            PRV_TGF_POLICER_MNG_CNTR_SET0_E,
                                            PRV_TGF_POLICER_MNG_CNTR_DROP_E,
                                            &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL1_STRING_MAC(
        duLowCntr, prvTgfMngCntrDrop.duMngCntr.l[0],
        "prvTgfMngCntrDrop.duMngCntr.l[0] = %d\n",
        prvTgfMngCntrDrop.duMngCntr.l[0]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        duHiCntr, prvTgfMngCntrDrop.duMngCntr.l[1],
        "prvTgfMngCntrDrop.duMngCntr.l[1] = %d\n",
        prvTgfMngCntrDrop.duMngCntr.l[1]);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        packetMngCntr, prvTgfMngCntrDrop.packetMngCntr,
        "prvTgfMngCntrDrop.packetMngCntr = %d\n",
        prvTgfMngCntrDrop.packetMngCntr);
}


/**
* @internal prvTgfPolicerTxEthTriggerCheck function
* @endinternal
*
* @brief   Check Tx Triggers
*
* @param[in] vfdArray[]               - VFD array with expected results
* @param[in] vfdNum                   - VFD number
*                                       None
*/
static GT_VOID prvTgfPolicerTxEthTriggerCheck
(
    IN GT_U8                    vfdNum,
    IN TGF_VFD_INFO_STC         vfdArray[]
)
{
    GT_STATUS                       rc;
    GT_U8                           queue    = 0;
    GT_BOOL                         getFirst = GT_TRUE;
    GT_U32                          buffLen  = 0x600;
    GT_U32                          packetActualLength = 0;
    static  GT_U8                   packetBuff[0x600] = {0};
    TGF_NET_DSA_STC                 rxParam;
    GT_BOOL                         triggered;
    GT_U8                           devNum;


    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficGeneratorRxInCpuGet %d", TGF_PACKET_TYPE_CAPTURE_E);

    /* Check that the pattern is equal to vfdArray */
    rc = tgfTrafficGeneratorTxEthTriggerCheck(packetBuff,
                                              buffLen,
                                              vfdNum,
                                              vfdArray,
                                              &triggered);
    UTF_VERIFY_EQUAL2_STRING_MAC(rc, GT_OK, "Error in tgfTrafficGeneratorTxEthTriggerCheck %d, %d", buffLen, vfdNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, triggered,
                                 "Packet VLAN TAG0 pattern is wrong");
}


/**
* @internal prvTgfPolicerCheckResults function
* @endinternal
*
* @brief   Check Tx Triggers
*
* @param[in] policerStage             - policer stage
* @param[in] up                       - User Priority
*
* @param[out] expectedBytesPtr         - Number of received bytes
*                                       None
*/
static GT_VOID prvTgfPolicerCheckResults
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT   policerStage,
    IN GT_U32                           up,
    OUT GT_U32                          *expectedBytesPtr
)
{
    GT_U32                              portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC       portCntrs;
    TGF_VFD_INFO_STC                    vfdArray[1];
    CPSS_PORT_EGRESS_CNTR_STC           egrCntr;
    GT_STATUS                           rc = GT_OK;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* check ETH counters */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfReadPortCountersEth FAILED, rc = [%d]", rc);
        }

        if (PRV_TGF_SEND_PORT_NUM_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        if (PRV_TGF_RECEIVE_PORT_NUM_CNS == portIter)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");

            *expectedBytesPtr = (portCntrs.goodOctetsSent.l[0] * prvTgfBurstCount);
        }
    }

    /* Configure vfdArray */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 12;
    vfdArray[0].cycleCount = 4;
    vfdArray[0].patternPtr[0] = 0x81;
    vfdArray[0].patternPtr[1] = 0x00;
    vfdArray[0].patternPtr[2] = (GT_U8)(up << 5);
    vfdArray[0].patternPtr[3] = (GT_U8)prvTgfPacketVlanTagPart.vid;


    prvTgfPolicerTxEthTriggerCheck(1, vfdArray);

    /* In Egress Policer stage, the egress counters show the result before
       Egress Policer Remark */
    if (policerStage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
            /* Read Egress counters */
            rc = prvTgfPortEgressCntrsGet(prvTgfEgressCntrSetNum, &egrCntr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet: %d", prvTgfDevNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, egrCntr.outUcFrames, "The number of UC packets is not as expected %d", prvTgfBurstCount);
    }
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPolicerQosRemarkingVlanConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfPolicerQosRemarkingVlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_PORT_EGRESS_CNTR_STC           egrCntr;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 3 with ports [0,1,2,3], TAG0 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d",
                                 prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:22:22:22:22, VLAN 3, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* Read Egress counters */
    rc = prvTgfPortEgressCntrsGet(prvTgfEgressCntrSetNum, &egrCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet: %d", prvTgfDevNum);

    cpssOsTimerWkAfter(10);
}

/**
* @internal prvTgfPolicerQosRemarkingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] policerStage             - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerQosRemarkingConfigurationRestore
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_QOS_ENTRY_STC                  portQosCfg;
    PRV_TGF_COS_PROFILE_STC             cosProfile;
    PRV_TGF_POLICER_QOS_PARAM_STC       egressQosParam;
    GT_U32                              dpInd;
    CPSS_PORT_EGRESS_CNTR_STC           egrCntr;
    CPSS_INTERFACE_INFO_STC             interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC          lookupCfg;
    GT_BOOL                 useQosProfile = GT_TRUE;
    GT_BOOL                 usePortMode = GT_TRUE;

    /* AUTODOC: RESTORE CONFIGURATION: */

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        usePortMode = GT_FALSE;
    }

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        useQosProfile = GT_FALSE;
    }

    /* AUTODOC: set the conformance level as GREEN */
    prvTgfPolicerConformanceLevelForce(0);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN 3 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate");

    /* AUTODOC: restore default VlanId for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS], 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

     /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* Restore Policer and QOS Configurations */

    /* AUTODOC: disable metering */
    rc = prvTgfPolicerMeteringEnableSet(policerStage, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable metering per source port 0 */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, policerStage,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortMeteringEnableSet: %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS]);

    if (usePortMode == GT_TRUE)
    {
        /* reset Policer Global stage mode - PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E by default */
        rc = prvTgfPolicerStageMeterModeSet(policerStage, PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "pssDxChPolicerStageMeterModeSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: reset metered packet size mode as L1_INCLUDE */
    rc = prvTgfPolicerPacketSizeModeSet(policerStage,
            CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerPacketSizeModeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore default counting mode */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, policerStage,
                                      prvTgfPolicerCntrModeDefaultArr[policerStage]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush internal Write Back Cache of counting entries */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(policerStage);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingWriteBackCacheFlush: %d", prvTgfDevNum);


    /* AUTODOC: set default counter color mode to CL */
    rc = prvTgfPolicerCountingColorModeSet(policerStage, PRV_TGF_POLICER_COLOR_COUNT_CL_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingColorModeSet: %d", prvTgfDevNum);


    if (policerStage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* Ingress stages */

        if (useQosProfile == GT_TRUE)
        {
            rc = prvTgfPolicerQosRemarkingEntrySet(policerStage, prvTgfQosProfileIndex, 0, 0, 0);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosRemarkingEntrySet: %d", prvTgfDevNum);


            cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));
            cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
            cosProfile.userPriority   = 0;
            cosProfile.trafficClass   = 0;
            cosProfile.dscp           = 0x0;
            cosProfile.exp            = 0;

            rc = prvTgfCosProfileEntrySet((GT_U8)prvTgfQosProfileIndex, &cosProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);

            rc = prvTgfCosProfileEntrySet((GT_U8)prvTgfYellowQosTableRemarkIndex, &cosProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);

            rc = prvTgfCosProfileEntrySet((GT_U8)prvTgfRedQosTableRemarkIndex, &cosProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);

            rc = prvTgfCosProfileEntrySet((GT_U8)prvTgfGreenQosTableRemarkIndex, &cosProfile);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d", prvTgfDevNum);
        }
        else
        {

        }
        /* Configure Port QoS attributes */
        portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        portQosCfg.qosProfileId     = 0;

        rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS],
                                       &portQosCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet: %d", prvTgfDevNum);

        /* Configure Port to Trust L2 */
        rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS],
                                          CPSS_QOS_PORT_TRUST_L2_E);

        /* restore Egress Counter configuration */
        rc = prvTgfPortEgressCntrModeSet(prvTgfEgressCntrSetNum, 0, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS],
                                         0, 0, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrModeSet: %d", prvTgfDevNum);


        /* Clear Egress counters */
        rc = prvTgfPortEgressCntrsGet(prvTgfEgressCntrSetNum, &egrCntr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortEgressCntrsGet: %d", prvTgfDevNum);
    }
    else
    {
        /* Egress stage */

        /* AUTODOC: disable Remarking for conforming packets */
        rc = prvTgfPolicerQosUpdateEnableSet(GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosUpdateEnableSet %d", prvTgfDevNum);


        /* AUTODOC: configure remark model to TC */
        rc = prvTgfPolicerL2RemarkModelSet(policerStage, PRV_TGF_POLICER_L2_REMARK_MODEL_TC_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerL2RemarkModelSet %d", prvTgfDevNum);

        egressQosParam.dp = CPSS_DP_GREEN_E;
        egressQosParam.dscp = 0x0;
        egressQosParam.exp = 0;
        egressQosParam.up = 0;
        egressQosParam.tc = 0;

        /* AUTODOC: clear QoS remarking table */
        for (dpInd = CPSS_DP_GREEN_E; dpInd <= CPSS_DP_RED_E; dpInd++)
        {
            rc = prvTgfPolicerEgressQosRemarkingEntrySet(PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                                         prvTgfPacketVlanTagPart.pri,
                                                         dpInd ,
                                                         &egressQosParam);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerEgressQosRemarkingEntrySet: %d", prvTgfDevNum);
        }

        /* clear PCL configuration table */
        cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

        interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.hwDevNum     =
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS];
        interfaceInfo.devPort.portNum    =
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS];

        lookupCfg.enableLookup           = GT_FALSE;
        lookupCfg.lookupType             = PRV_TGF_PCL_LOOKUP_TYPE_SINGLE_E;
        lookupCfg.dualLookup             = GT_FALSE;
        lookupCfg.externalLookup         = GT_FALSE;
        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
        lookupCfg.groupKeyTypes.ipv6Key =
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

        /* AUTODOC: clear PCL configuration table */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

        /* AUTODOC: disable egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet: %d", prvTgfDevNum);


        /* AUTODOC: invalidate PCL rule 1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);
    }
}


/**
* @internal prvTgfPolicerIngressQosRemarkingByEntryTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerIngressQosRemarkingByEntryTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              stage;
    GT_U32                              expectedBytes = 0;
    PRV_TGF_COS_PROFILE_STC             cosProfile;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT meterTbParams;


    /* AUTODOC: GENERATE TRAFFIC: */
    if(GT_FALSE == prvTgfPolicerStageCheck(policerStage))
    {
        /* the device not supports this stage */
        return;
    }

    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG1_MAC("                    POLICER_STAGE_INGRESS_%d\n",
            policerStage);


    /* Disable Metering and Counting on all stages */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E; stage < PRV_TGF_POLICER_STAGE_NUM; stage++)
    {
        if(GT_FALSE == prvTgfPolicerStageCheck(stage))
        {
            /* the device not supports this stage */
            continue;
        }

        /* AUTODOC: disable policer metering for all stages */
        rc = prvTgfPolicerMeteringEnableSet(stage, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

        /* AUTODOC: disable policer counting for all stages */
        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
     * 1.1 Ingress Remarking By Entry per Source Port Metering with RED packets
     */
    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("1.1 Ingress Remarking By Entry per Source Port Metering"
                     " with RED packets\n");

    /* configure QoS Profile with index 5 - for Mark by entry command */
    cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));
    cosProfile.dropPrecedence = CPSS_DP_RED_E;
    cosProfile.userPriority   = 7;
    cosProfile.trafficClass   = 4;
    cosProfile.dscp           = 0x32;
    cosProfile.exp            = 0;

    meterTbParams.srTcmParams.cir = 1;
    meterTbParams.srTcmParams.cbs = 1;
    meterTbParams.srTcmParams.ebs = 10304; /* 1 yellow packet */

    prvTgfPolicerQosRemarkingConfigurationSet(policerStage,
            PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E,
            PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E,
            &meterTbParams, &cosProfile, &cosProfile,
            0, 0, 0, 0, 0, NULL);
    /* AUTODOC: enable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to PORT */
    /* AUTODOC: set inress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=REMARK_BY_ENTRY */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=1, cbs=1, ebs=10304 */
    /* AUTODOC: set Cos profile entry 5: */
    /* AUTODOC:   UP=7, TC=4, DSCP=0x32, EXP=0, DP=RED */

    /* AUTODOC: set the conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=7 */
    prvTgfPolicerCheckResults(policerStage, cosProfile.userPriority,
                              &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   YELLOW - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   RED - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, expectedBytes, 0, prvTgfBurstCount);
    prvTgfPolicerManagementCountersRedCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);


    /* -------------------------------------------------------------------------
     * 1.2 Ingress Remarking by Entry per Source Port Metering with GREEN packets
     */
    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("1.2 Ingress Remarking by Entry per Source Port Metering"
                     " with GREEN packets\n");


    /* configure QoS Profile with index 5 - for Mark by entry command */
    cpssOsMemSet(&cosProfile, 0, sizeof(cosProfile));
    cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
    cosProfile.userPriority   = 4;
    cosProfile.trafficClass   = 1;
    cosProfile.dscp           = 0x0;
    cosProfile.exp            = 0;


    meterTbParams.srTcmParams.cir = 1;
    meterTbParams.srTcmParams.cbs = 1;
    meterTbParams.srTcmParams.ebs = 1; /* 1 red packet */


    prvTgfPolicerQosRemarkingConfigurationSet(policerStage,
            PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E,
            PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E,
            &meterTbParams, &cosProfile, &cosProfile,
            0, 0, 0, 0, 0, NULL);
    /* AUTODOC: enable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to PORT */
    /* AUTODOC: set inress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=REMARK_BY_ENTRY */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=1, cbs=1, ebs=1 */
    /* AUTODOC: set Cos profile entry 5: */
    /* AUTODOC:   UP=4, TC=1, DSCP=0x0, EXP=0, DP=GREEN */

    /* AUTODOC: set the conformance level as red */
    prvTgfPolicerConformanceLevelForce(2);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=4 */
    prvTgfPolicerCheckResults(policerStage, cosProfile.userPriority,
                              &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   YELLOW - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   RED - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersRedCheck(policerStage, expectedBytes, 0, prvTgfBurstCount);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);

    prvTgfPolicerCountersReset(policerStage);
}


/**
* @internal prvTgfPolicerIngressQosRemarkingByTableTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerIngressQosRemarkingByTableTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              stage;
    GT_U32                              expectedBytes = 0;
    PRV_TGF_COS_PROFILE_STC             yellowCosProfile;
    PRV_TGF_COS_PROFILE_STC             redCosProfile;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT meterTbParams;

    /* AUTODOC: GENERATE TRAFFIC: */
    if(GT_FALSE == prvTgfPolicerStageCheck(policerStage))
    {
        /* the device not supports this stage */
        return;
    }

    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG1_MAC("                    POLICER_STAGE_INGRESS_%d\n",
            policerStage);


    /* Disable Metering and Counting on all stages */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E; stage < PRV_TGF_POLICER_STAGE_NUM; stage++)
    {
        if(GT_FALSE == prvTgfPolicerStageCheck(stage))
        {
            /* the device not supports this stage */
            continue;
        }

        /* AUTODOC: disable policer metering for all stages */
        rc = prvTgfPolicerMeteringEnableSet(stage, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

        /* AUTODOC: disable policer counting for all stages */
        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
     * 1.1 Ingress Remarking By Table per Source Port Metering with RED packets
     */
    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("1.1 Ingress Remarking By Table per Source Port Metering"
                     " with RED packets\n");


    /* Set packet QoS values, that we should get after remarking */

    /* configure QoS Profile - for yellow */
    cpssOsMemSet(&yellowCosProfile, 0, sizeof(yellowCosProfile));
    yellowCosProfile.dropPrecedence = CPSS_DP_GREEN_E;
    yellowCosProfile.userPriority   = 2;
    yellowCosProfile.trafficClass   = 7;
    yellowCosProfile.dscp           = 0x32;
    yellowCosProfile.exp            = 0;

    /* configure QoS Profile - for red */
    cpssOsMemSet(&redCosProfile, 0, sizeof(redCosProfile));
    redCosProfile.dropPrecedence = CPSS_DP_RED_E;
    redCosProfile.userPriority   = 7;
    redCosProfile.trafficClass   = 0;
    redCosProfile.dscp           = 0x11;
    redCosProfile.exp            = 0;


    meterTbParams.srTcmParams.cir = 1;
    meterTbParams.srTcmParams.cbs = 1;
    meterTbParams.srTcmParams.ebs = 10304; /* 1 yellow packet */

    prvTgfPolicerQosRemarkingConfigurationSet(policerStage,
            PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E,
            PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E,
            &meterTbParams, &yellowCosProfile, &redCosProfile,
            0, 0, 0, 0, 0, NULL);
    /* AUTODOC: enable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to PORT */
    /* AUTODOC: set inress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=NON_CONFORM_CMD_REMARK */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=1, cbs=1, ebs=10304 */
    /* AUTODOC: set Cos profile entry 10: */
    /* AUTODOC:   UP=2, TC=7, DSCP=0x32, EXP=0, DP=GREEN */
    /* AUTODOC: set Cos profile entry 20: */
    /* AUTODOC:   UP=7, TC=0, DSCP=0x11, EXP=0, DP=RED */
    /* AUTODOC: add policer relative QoS remarking entry 5: */
    /* AUTODOC:   yellowIndex=10, redIndex=20 */
    /* AUTODOC: configure QoS attributes for port 0: */
    /* AUTODOC:   qosProfileId=5, assignPrecedence=SOFT */
    /* AUTODOC:   enable ModifyDscp and ModifyUp */
    /* AUTODOC: set trust mode for port 0 as NO_TRUST */

    /* AUTODOC: set the conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=1 */
    prvTgfPolicerCheckResults(policerStage, yellowCosProfile.userPriority,
                              &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   YELLOW - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   RED - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, expectedBytes, 0, prvTgfBurstCount);
    prvTgfPolicerManagementCountersRedCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);

    prvTgfPolicerCountersReset(policerStage);
}


/**
* @internal prvTgfPolicerEgressQosRemarkingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerEgressQosRemarkingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT     meterTbParams;
    PRV_TGF_POLICER_QOS_PARAM_STC           qosParam;
    GT_U32                                  stage;
    GT_U32                                  expectedBytes;
    PRV_TGF_POLICER_STAGE_TYPE_ENT          policerStage =
                                                PRV_TGF_POLICER_STAGE_EGRESS_E;

    /* AUTODOC: GENERATE TRAFFIC: */
    if(GT_FALSE == prvTgfPolicerStageCheck(policerStage))
    {
        /* the device not supports this stage */
        return;
    }

    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("                      POLICER_STAGE_EGRESS\n");


    /* Disable Metering and Counting on all stages */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E; stage < PRV_TGF_POLICER_STAGE_NUM; stage++)
    {
        if(GT_FALSE == prvTgfPolicerStageCheck(stage))
        {
            /* the device not supports this stage */
            continue;
        }

        /* AUTODOC: disable policer metering for all stages */
        rc = prvTgfPolicerMeteringEnableSet(stage, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

        /* AUTODOC: disable policer counting for all stages */
        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);
    }
    /* -------------------------------------------------------------------------
     * 4.1 Egress Remarking with RED packets L2
     */
    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("4.1 Egress Remarking with YELLOW packets L2\n");
    PRV_UTF_LOG0_MAC("Expected results: RED Counters count the packets\n\n");

    /* AUTODOC: Remark YELLOW packet to RED */

    meterTbParams.srTcmParams.cir = 1;
    meterTbParams.srTcmParams.cbs = 1;
    meterTbParams.srTcmParams.ebs = 10304; /* 1 yellow packet */

    /* Configure Qos Remarking entries UP Remarking and CL is YELLOW */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    qosParam.up   = 1;
    qosParam.dscp = 0x20;
    qosParam.exp  = 0;
    qosParam.dp   = CPSS_DP_RED_E;

    /* set configuration for Egress YELLOW packets */
    prvTgfPolicerQosRemarkingConfigurationSet(policerStage,
                                              PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E,
                                              PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E,
                                              &meterTbParams,
                                              NULL, NULL,
                                              PRV_TGF_POLICER_REMARK_MODE_L2_E,
                                              PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E,
                                              PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                              prvTgfPacketVlanTagPart.pri,
                                              CPSS_DP_YELLOW_E,
                                              &qosParam);
    /* AUTODOC: disable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to FLOW */
    /* AUTODOC: set egress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=NON_CONFORM_CMD_NO_CHANGE */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=1, cbs=1, ebs=10304 */
    /* AUTODOC: set policer remarking model to L2_REMARK_MODEL_UP */
    /* AUTODOC: set egress Policer remarking entry: */
    /* AUTODOC:   tableType=TC_UP, paramValue=5, confLevel=DP_YELLOW */
    /* AUTODOC:   QoS params: UP=1, DSCP=0x20, EXP=0, DP=RED */

    prvTgfPolicerQosRemarkingPclConfigurationSet(CPSS_PCL_DIRECTION_EGRESS_E,
                                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS]);

    /* AUTODOC: disable egress QoS remarking for conforming packets */
    rc = prvTgfPolicerQosUpdateEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosUpdateEnableSet %d", prvTgfDevNum);

    /* AUTODOC: set the conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);


    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=1 */
    prvTgfPolicerCheckResults(policerStage, qosParam.up,
                              &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   YELLOW - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   RED - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersRedCheck(policerStage, expectedBytes, 0, prvTgfBurstCount);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);



    /* -------------------------------------------------------------------------
     * 4.1 Egress Remarking with RED packets L2
     */
    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("4.1 Egress Remarking with YELLOW packets L2\n");
    PRV_UTF_LOG0_MAC("Expected results: RED Counters count the packets\n\n");

    /* AUTODOC: Remark YELLOW packet to RED */

    meterTbParams.srTcmParams.cir = 1;
    meterTbParams.srTcmParams.cbs = 1;
    meterTbParams.srTcmParams.ebs = 10304; /* 1 yellow packet */

    /* Configure Qos Remarking entries UP Remarking and CL is YELLOW */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    qosParam.up   = 1;
    qosParam.dscp = 0x20;
    qosParam.exp  = 0;
    qosParam.dp   = CPSS_DP_RED_E;


    /* set configuration for Egress YELLOW packets */
    prvTgfPolicerQosRemarkingConfigurationSet(policerStage,
                                              PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E,
                                              PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E,
                                              &meterTbParams,
                                              NULL, NULL,
                                              PRV_TGF_POLICER_REMARK_MODE_L2_E,
                                              PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E,
                                              PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                              prvTgfPacketVlanTagPart.pri,
                                              CPSS_DP_YELLOW_E,
                                              &qosParam);
    /* AUTODOC: disable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to FLOW */
    /* AUTODOC: set egress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=NON_CONFORM_CMD_NO_CHANGE */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=1, cbs=1, ebs=10304 */
    /* AUTODOC: set policer remarking model to L2_REMARK_MODEL_UP */
    /* AUTODOC: set egress Policer remarking entry: */
    /* AUTODOC:   tableType=TC_UP, paramValue=5, confLevel=DP_YELLOW */
    /* AUTODOC:   QoS params: UP=1, DSCP=0x20, EXP=0, DP=RED */

    prvTgfPolicerQosRemarkingPclConfigurationSet(CPSS_PCL_DIRECTION_EGRESS_E,
                                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS]);

    /* AUTODOC: disable egress QoS remarking for conforming packets */
    rc = prvTgfPolicerQosUpdateEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosUpdateEnableSet %d", prvTgfDevNum);

    /* AUTODOC: set conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);


    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=1 */
    prvTgfPolicerCheckResults(policerStage, qosParam.up,
                              &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   YELLOW - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   RED - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersRedCheck(policerStage, expectedBytes, 0, prvTgfBurstCount);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);


    /* AUTODOC: Remark RED packet to GREEN */
    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("4.1 Egress Remarking with RED packets L2\n");
    PRV_UTF_LOG0_MAC("Expected results: GREEN Counters count the packets\n\n");

    meterTbParams.srTcmParams.cir = 1;
    meterTbParams.srTcmParams.cbs = 1;
    meterTbParams.srTcmParams.ebs = 1; /* 1 red packet */

    /* Configure Qos Remarking entries for UP Remarking and CL is RED */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    qosParam.up   = 7;
    qosParam.dscp = 0x20;
    qosParam.exp  = 0;
    qosParam.dp   = CPSS_DP_GREEN_E;

    /* set configuration for Egress YELLOW packets */
    prvTgfPolicerQosRemarkingConfigurationSet(policerStage,
                                              PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E,
                                              PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E,
                                              &meterTbParams,
                                              NULL, NULL,
                                              PRV_TGF_POLICER_REMARK_MODE_L2_E,
                                              PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E,
                                              PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                              prvTgfPacketVlanTagPart.pri,
                                              CPSS_DP_RED_E,
                                              &qosParam);
    /* AUTODOC: disable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to FLOW */
    /* AUTODOC: set egress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=NON_CONFORM_CMD_NO_CHANGE */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=1, cbs=1, ebs=1 */
    /* AUTODOC: set policer remarking model to L2_REMARK_MODEL_UP */
    /* AUTODOC: set egress Policer remarking entry: */
    /* AUTODOC:   tableType=TC_UP, paramValue=5, confLevel=DP_RED */
    /* AUTODOC:   QoS params: UP=7, DSCP=0x20, EXP=0, DP=GREEN */

    /* AUTODOC: set the conformance level as red */
    prvTgfPolicerConformanceLevelForce(2);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=7 */
    prvTgfPolicerCheckResults(policerStage, qosParam.up, &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   YELLOW - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   RED - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, expectedBytes, 0, prvTgfBurstCount);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersRedCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);


    /* AUTODOC: Remark GREEN packet to RED */

        PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("4.1 Egress Remarking with GREEN packets L2\n");
    PRV_UTF_LOG0_MAC("Expected results: RED Counters count the packets\n\n");

    meterTbParams.srTcmParams.cir = 1;
    meterTbParams.srTcmParams.cbs = 10304; /* 1 green packet */
    meterTbParams.srTcmParams.ebs = 1;

    /* Configure Qos Remarking entries for UP and CL is GREEN */
    cpssOsMemSet(&qosParam, 0, sizeof(qosParam));
    qosParam.up   = 2;
    qosParam.dscp = 0x20;
    qosParam.exp  = 0;
    qosParam.dp   = CPSS_DP_RED_E;

    /* set configuration for Egress RED packets */
    prvTgfPolicerQosRemarkingConfigurationSet(policerStage,
                                              PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E,
                                              PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E,
                                              &meterTbParams,
                                              NULL, NULL,
                                              PRV_TGF_POLICER_REMARK_MODE_L2_E,
                                              PRV_TGF_POLICER_L2_REMARK_MODEL_UP_E,
                                              PRV_TGF_POLICER_REMARK_TABLE_TYPE_TC_UP_E,
                                              prvTgfPacketVlanTagPart.pri,
                                              CPSS_DP_GREEN_E,
                                              &qosParam);
    /* AUTODOC: disable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to FLOW */
    /* AUTODOC: set egress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=NON_CONFORM_CMD_NO_CHANGE */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=1, cbs=10304, ebs=1 */
    /* AUTODOC: set policer remarking model to L2_REMARK_MODEL_UP */
    /* AUTODOC: set egress Policer remarking entry: */
    /* AUTODOC:   tableType=TC_UP, paramValue=5, confLevel=DP_GREEN */
    /* AUTODOC:   QoS params: UP=2, DSCP=0x20, EXP=0, DP=RED */

    /* Enable Remarking for conforming packets */
    rc = prvTgfPolicerQosUpdateEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerQosUpdateEnableSet %d", prvTgfDevNum);

    /* AUTODOC: set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=2 */
    prvTgfPolicerCheckResults(policerStage, qosParam.up, &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   YELLOW - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   RED - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersRedCheck(policerStage, expectedBytes, 0,
                                            prvTgfBurstCount);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);


    prvTgfPolicerCountersReset(policerStage);
}

/**
* @internal prvTgfPolicerIngressQosRemarkingByTableGreenTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPolicerIngressQosRemarkingByTableGreenTrafficGenerate
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              stage;
    GT_U32                              expectedBytes = 0;
    PRV_TGF_COS_PROFILE_STC             greenCosProfile;
    PRV_TGF_COS_PROFILE_STC             yellowCosProfile;
    PRV_TGF_COS_PROFILE_STC             redCosProfile;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT meterTbParams;

    /* AUTODOC: GENERATE TRAFFIC: */
    if(GT_FALSE == prvTgfPolicerStageCheck(policerStage))
    {
        /* the device not supports this stage */
        return;
    }

    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG1_MAC("                    POLICER_STAGE_INGRESS_%d\n",
            policerStage);


    /* Disable Metering and Counting on all stages */
    for (stage = PRV_TGF_POLICER_STAGE_INGRESS_0_E; stage < PRV_TGF_POLICER_STAGE_NUM; stage++)
    {
        if(GT_FALSE == prvTgfPolicerStageCheck(stage))
        {
            /* the device not supports this stage */
            continue;
        }

        /* AUTODOC: disable policer metering for all stages */
        rc = prvTgfPolicerMeteringEnableSet(stage, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);

        /* AUTODOC: disable policer counting for all stages */
        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d", prvTgfDevNum);
    }


    /* -------------------------------------------------------------------------
     * 1.1 Ingress Remarking By Table per Source Port Metering with RED packets
     */
    PRV_UTF_LOG0_MAC("================================"
                     "================================\n");
    PRV_UTF_LOG0_MAC("1.1 Ingress Remarking By Table per Source Port Metering"
                     " with RED packets\n");


    /* Set packet QoS values, that we should get after remarking */

    /* configure QoS Profile - for yellow */
    cpssOsMemSet(&greenCosProfile, 0, sizeof(greenCosProfile));
    greenCosProfile.dropPrecedence = CPSS_DP_GREEN_E;
    greenCosProfile.userPriority   = 7;
    greenCosProfile.trafficClass   = 7;
    greenCosProfile.dscp           = 0x33;
    greenCosProfile.exp            = 0;

    /* configure QoS Profile - for yellow */
    cpssOsMemSet(&yellowCosProfile, 0, sizeof(yellowCosProfile));
    yellowCosProfile.dropPrecedence = CPSS_DP_YELLOW_E;
    yellowCosProfile.userPriority   = 2;
    yellowCosProfile.trafficClass   = 3;
    yellowCosProfile.dscp           = 0x22;
    yellowCosProfile.exp            = 0;

    /* configure QoS Profile - for red */
    cpssOsMemSet(&redCosProfile, 0, sizeof(redCosProfile));
    redCosProfile.dropPrecedence = CPSS_DP_RED_E;
    redCosProfile.userPriority   = 0;
    redCosProfile.trafficClass   = 0;
    redCosProfile.dscp           = 0x11;
    redCosProfile.exp            = 0;


    meterTbParams.srTcmParams.cir = 10000;
    meterTbParams.srTcmParams.cbs = 10304;
    meterTbParams.srTcmParams.ebs = 10304; /* 1 green packet */

    prvTgfPolicerQosRemarkingConfigurationExtSet(
        policerStage,
        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E,
        PRV_TGF_POLICER_NON_CONFORM_CMD_REMARK_E,
        &meterTbParams, &yellowCosProfile, &redCosProfile,
        0, 0, 0, 0, 0, NULL,
        GT_TRUE /*greenRemarkingSupport*/, &greenCosProfile);
    /* AUTODOC: enable port metering trigger on port 0 */
    /* AUTODOC: set policer global stage mode to PORT */
    /* AUTODOC: set inress Policer Entry 0: */
    /* AUTODOC:   cntEntryIndex=0, qosProfile=5 */
    /* AUTODOC:   modifyUp=modifyDscp=modifyDp=modifyTc=Enable */
    /* AUTODOC:   yellowPcktCmd=redPcktCmd=NON_CONFORM_CMD_REMARK */
    /* AUTODOC:   meterMode=MODE_SR_TCM, remarkMode=MODE_L2 */
    /* AUTODOC:   tbParams cir=10000, cbs=10304, ebs=10304 */
    /* AUTODOC: set Cos profile entry 15: green */
    /* AUTODOC:   UP=7, TC=7, DSCP=0x33, EXP=0, DP=RED */
    /* AUTODOC: set Cos profile entry 20: yellow */
    /* AUTODOC:   UP=2, TC=3, DSCP=0x22, EXP=0, DP=GREEN */
    /* AUTODOC: set Cos profile entry 10: red */
    /* AUTODOC:   UP=0, TC=0, DSCP=0x11, EXP=0, DP=RED */
    /* AUTODOC: add policer relative QoS remarking entry 5: */
    /* AUTODOC:   green=15, yellowIndex=10, redIndex=20 */
    /* AUTODOC: configure QoS attributes for port 0: */
    /* AUTODOC:   qosProfileId=5, assignPrecedence=SOFT */
    /* AUTODOC:   enable ModifyDscp and ModifyUp */
    /* AUTODOC: set trust mode for port 0 as NO_TRUST */

    /* AUTODOC: set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=5 */
    prvTgfPolicerTestPacketSend(&prvTgfPacketInfo);

    /* AUTODOC: verify traffic on port 1: */
    /* AUTODOC:   DA=00:00:22:22:22:22, SA=00:00:11:11:11:11, VID=3, UP=1 */
    prvTgfPolicerCheckResults(policerStage, greenCosProfile.userPriority,
                              &expectedBytes);

    /* print the value of specified Management and Policy Counters */
    prvTgfPolicerCountersPrint(policerStage);

    /* AUTODOC: check Management Counters: */
    /* AUTODOC:   GREEN - duMngCntr=64, pcktMngCntr=1 */
    /* AUTODOC:   YELLOW - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   RED - duMngCntr=0, pcktMngCntr=0 */
    /* AUTODOC:   DROP - duMngCntr=0, pcktMngCntr=0 */
    prvTgfPolicerManagementCountersGreenCheck(policerStage, expectedBytes, 0, prvTgfBurstCount);
    prvTgfPolicerManagementCountersYellowCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersRedCheck(policerStage, 0, 0, 0);
    prvTgfPolicerManagementCountersDropCheck(policerStage, 0, 0, 0);

    prvTgfPolicerCountersReset(policerStage);
}



/**
* @internal prvTgfPolicerWSRateRatioRateTypeTest function
* @endinternal
*
* @brief   test checks different cir and cbs combinations with 
*          different rateTypes
* */ 
GT_VOID prvTgfPolicerWSRateRatioRateTypeTest
(
    GT_VOID
)
{
    GT_U32 ii = 0;
    GT_U32 currCir;
    GT_U32 currCbs;
    ratePair *ratesCirAndCbs; /*pair of cir and cbs*/

    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            /* 100G egress port supports rate 3 and below, not support rate 3 maximum which is 131G*/
            ratesCirAndCbs = &(prvTgfCirAndCbs_100G_Array[0]);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            /* 10G egress port supports rate 2 and below, not support rate 2 maximum which is 13G*/
            ratesCirAndCbs = &(prvTgfCirAndCbs_10G_Array[0]);
        default: 
            /* 100M egress port supports rate 1 and below, not support rate 1 maximum which is 1.3G*/
            ratesCirAndCbs = &(prvTgfCirAndCbs_100M_Array[0]);
            break;
    }

    /* AUTODOC: Test configurations */
    prvTgfPolicerWSRateRatioRateTypeSet();

    while ((ratesCirAndCbs->cir != 0xffffffff) && (ratesCirAndCbs->cbs != 0xffffffff))
    {
        currCir = ratesCirAndCbs->cir;
        currCbs = ratesCirAndCbs->cbs;
        PRV_UTF_LOG4_MAC("\nset option %d: rateType: %d, cir: %d, cbs: %d\n",ii, (ii/4), currCir, currCbs);

        /* AUTODOC: Test traffic and checks */
        prvTgfPolicerWSRateRatioRateTypeGenerateTraffic(currCir,currCbs);
        ratesCirAndCbs++;
        ii++;
    }

    /* AUTODOC: Restore configurations */
    prvTgfPolicerWSRateRatioRateTypeRestore();

}

/**
* @internal prvTgfPolicerWSRateRatioRateTypeSet function
* @endinternal
*
* @brief   set test configuration
*/
GT_VOID prvTgfPolicerWSRateRatioRateTypeSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 stage = PRV_TGF_POLICER_STAGE_EGRESS_E;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 3 with ports [0,1,2,3], TAG0 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: %d",
                                 prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:22:22:22:22, VLAN 3, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* set policer metering CIR and CBS */

    /* set metring configuration*/
    /* AUTODOC: Save stage metering enable for restore */
    rc = prvTgfPolicerMeteringEnableGet(prvTgfDevNum,
                                        stage,
                                        &saveStageMeterEnable[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveStageMeterEnable[stage]);

    /* AUTODOC: Enable stage for metering */
    rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage, GT_TRUE);

    /* AUTODOC: set Bytes based meter resolution algorithm */
    rc = prvTgfPolicerMeterResolutionSet(stage,
            PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeterResolutionSet: %d", prvTgfDevNum);

    /* AUTODOC: Save port metering enable for restore */
    rc = prvTgfPolicerPortMeteringEnableGet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                            &savePortMeterEnable[stage]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableGet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                 savePortMeterEnable[stage]);

    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                            GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                 GT_TRUE);

    /* AUTODOC: Save metering mode for restore */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum,
                                        stage,
                                        &saveMeterMode[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveMeterMode[stage]);

    /* AUTODOC: Set metering mode for "port" mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                       PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* save metering entry for restore */
    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                          stage,
                          prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                          &saveMeteringEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerEntryGet");


    rc = prvTgfDevPortMacCountersClearOnReadGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],&saveClearOnReadMacCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfDevPortMacCountersClearOnReadGet");

    rc = prvTgfDevPortMacCountersClearOnReadSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfDevPortMacCountersClearOnReadSet");
}



/**
* @internal prvTgfPolicerWSGenerateTraffic function
* @endinternal
*
* @brief   Configure, start WS traffic from port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (Pointer to) packet data
*                                       None
*/
static GT_STATUS prvTgfPolicerWSGenerateTraffic
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  TGF_PACKET_STC                 *packetInfoPtr
)
{
    GT_STATUS   rc;

    /* reset ingress port counters  */
    rc = prvTgfResetCountersEth(devNum, portNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("prvTgfResetCountersEth FAILED:rc-%d,devNum-%d,portNum-%d\n",
                                                    rc, devNum, portNum);
        return rc;
    }

    /* reset egress port counters  */
    rc = prvTgfResetCountersEth(devNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS]);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("prvTgfResetCountersEth FAILED:rc-%d,devNum-%d,portNum-%d\n",
                                                    rc, devNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS]);
        return rc;
    }


    /* setup Packet */
    rc = prvTgfSetTxSetupEth(devNum, packetInfoPtr, 1, 0, NULL);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("prvTgfSetTxSetupEth FAILED:rc-%d,devNum-%d\n",
                                                    rc, devNum);
        return rc;
    }

    /* configure WS mode */
    rc = prvTgfTxModeSetupEth(devNum, portNum,
                              PRV_TGF_TX_MODE_CONTINUOUS_E,
                              PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                              0);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("prvTgfSetTxSetupEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    rc, devNum, portNum);
        return rc;
    }

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(devNum, portNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("prvTgfStartTransmitingEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    rc, devNum, portNum);
        return rc;
    }

    /* wait to get stable traffic */
    cpssOsTimerWkAfter(10);

    return GT_OK;
}

/**
* @internal prvTgfPolicerWSTrafficStop function
* @endinternal
*
* @brief   Configure, stop WS traffic from port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*/
static GT_STATUS prvTgfPolicerWSTrafficStop
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS   rc;

    /* ----- stop send Packet from port portNum -------- */
    rc = prvTgfStopTransmitingEth(devNum, portNum);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("prvTgfStopTransmitingEth FAILED:rc-%d,devNum-%d,portNum-%d\n",
                                                    rc, devNum, portNum);
        return rc;
    }

    /* wait some time to guarantee that traffic stopped */
    cpssOsTimerWkAfter(10);

    /* configure default mode */
    rc = prvTgfTxModeSetupEth(devNum, portNum,
                                PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                0);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("prvTgfTxModeSetupEth FAILED:rc-%d,devNum-%d,portNum-%d\n",
                            rc, devNum, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvTgfPolicerWSRateRatioRateTypeGenerateTraffic
*           function
* @endinternal
*
* @brief   generate ws traffic and check port rate
*/
GT_VOID prvTgfPolicerWSRateRatioRateTypeGenerateTraffic
(
    GT_U32 cir,
    GT_U32 cbs
)
{

    GT_STATUS   rc;
    GT_U32 ratePacketsPerSecond; /* expected packet rate in pps*/
    GT_U32 txRate; /* actual egress port rate*/
    GT_U32 rateDiff;
    GT_U64 counter;
    GT_U32 inPortsIdxArr[1] = {PRV_TGF_SEND_PORT_NUM_CNS};
    GT_U32 inRateArr[1] ={0}; /* actual ingress port rate*/
    GT_U32 l1PacketSize;
    GT_U32 packetLen = prvTgfPacketInfo.totalLen + 4/* CRC*/;
    PRV_TGF_POLICER_ENTRY_STC   meteringEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U32 stage = PRV_TGF_POLICER_STAGE_EGRESS_E;


    /* --------------- set metering entry --------------- */
    cpssOsMemSet(&meteringEntry, 0, sizeof(meteringEntry));

    /* AUTODOC: create metering entry per egress Port */
    cpssOsMemSet(&meteringEntry, 0, sizeof(meteringEntry));
    meteringEntry.meterColorMode     = CPSS_POLICER_COLOR_BLIND_E;
    meteringEntry.remarkMode         = PRV_TGF_POLICER_REMARK_MODE_L2_E;
    meteringEntry.meterMode          = PRV_TGF_POLICER_METER_MODE_SR_TCM_E;
    meteringEntry.mngCounterSet      = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    meteringEntry.tbParams.srTcmParams.cir = cir;
    meteringEntry.tbParams.srTcmParams.cbs = cbs;
    meteringEntry.tbParams.srTcmParams.ebs = cbs;
    meteringEntry.modifyUp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyDscp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.modifyDp           = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meteringEntry.greenPcktCmd       = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meteringEntry.yellowPcktCmd      = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
    meteringEntry.redPcktCmd         = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;

    rc = prvTgfPolicerEntrySet(
        stage,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
        &meteringEntry, &tbParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerEntrySet");


    /* calculate expected results of port rate*/
    /* add 20 bytes IPG and preamble to get L1 byte size */
    l1PacketSize = (packetLen + 20) * 8;
    /* kbps -> pps*/
    ratePacketsPerSecond = (cir/l1PacketSize);
    ratePacketsPerSecond = (1000*ratePacketsPerSecond);
    PRV_UTF_LOG3_MAC("expected rate [pps] of %d, cir is  %d [kbps] on egress port %d\n",ratePacketsPerSecond, cir,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS]);
    /* reset Tx counter by read it */
    rc = prvTgfPortMacCounterGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS], CPSS_GOOD_OCTETS_SENT_E, &counter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPortMacCounterGet %d", prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS]);


    /* start WS */
    rc = prvTgfPolicerWSGenerateTraffic(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS],&prvTgfPacketInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"prvTgfPolicerWSGenerateTraffic FAILED: devNum-%d, portNum-%d\n",
                                                  prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS]);


    /* get ingress port rate. validate it's WS*/
     prvTgfCommonPortsRxRateGet(inPortsIdxArr, 1/* number of ingress ports*/, 1000, inRateArr);
     rc = prvTgfPortWsRateCheck(inPortsIdxArr[0], inRateArr[0], packetLen);
     UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"prvTgfPortWsRateCheck FAILED: devNum-%d, portNum-%d, Rx rate %d\n",
                                  prvTgfDevNum,prvTgfPortsArray[inPortsIdxArr[0]],inRateArr[0] );


    /* AUTODOC get egress port rate. validate it's equal to the configured cir*/
    prvTgfCommonPortTxRateGet(PRV_TGF_RECEIVE_PORT_NUM_CNS, 1000, &txRate);
    rateDiff = prvTgfCommonDiffInPercentCalc(txRate, ratePacketsPerSecond);
    PRV_UTF_LOG2_MAC("txRate [pps] is %d, diff is %d %\n",txRate, rateDiff);
    if (rateDiff > 10) /* allow 10% difference. 5% are appeared sometimes */
    {
        /* test failure */
        UTF_VERIFY_EQUAL4_STRING_MAC(0, rateDiff, "port %d actual tx rate %d, not as expected %d, rateDiff is %d %\n",prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],txRate,ratePacketsPerSecond, rateDiff);
    }


    /* stop WS*/
    rc = prvTgfPolicerWSTrafficStop(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"prvTgfPolicerWSTrafficStop FAILED: devNum-%d, portNum-%d\n",
                                                  prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_NUM_CNS]);

}


GT_VOID prvTgfPolicerWSRateRatioRateTypeRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 stage = PRV_TGF_POLICER_STAGE_EGRESS_E;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U32 rate;

    /* AUTODOC: Restore stage metering enable */
    rc = prvTgfPolicerMeteringEnableSet(stage,
                                        saveStageMeterEnable[stage]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 saveStageMeterEnable[stage]);

    /* AUTODOC: Restore port metering enable */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                            savePortMeterEnable[stage]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
                                 savePortMeterEnable[stage]);

    /* AUTODOC: Restore metering mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        saveMeterMode[stage]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerMeteringEnableSet: %d, %d",
                                 stage,
                                 saveMeterMode[stage]);
    /* AUTODOC: Restore metering entry */
    rc = prvTgfPolicerEntrySet(
        stage,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],
        &saveMeteringEntry, &tbParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPolicerEntrySet");

    /*reset MAC counters*/
    prvTgfCommonPortTxRateGet(PRV_TGF_RECEIVE_PORT_NUM_CNS, 1000, &rate);

    rc = prvTgfDevPortMacCountersClearOnReadSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_NUM_CNS],saveClearOnReadMacCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfDevPortMacCountersClearOnReadSet");

}


