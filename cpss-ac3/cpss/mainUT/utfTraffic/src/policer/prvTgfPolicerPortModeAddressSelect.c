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
* @file prvTgfPolicerPortModeAddressSelect.c
*
* @brief Specific Policer features testing
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>


#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <policer/prvTgfPolicerPortModeAddressSelect.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>

/* default VLAN Id */
#define PRV_TGF_VLANID_2_CNS            2

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* default number of packets to send */
static GT_U32   prvTgfBurstCount   = 2;

/* number of iterations in the test */
#define PRV_ADDR_SELECT_ITERAT_NUM_CNS 4

/* default meter mode: port or flow */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT defaultMeterMode;

/* default memory mode */
static PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT defaultMemMode;
static GT_U32 orig_numEntriesIngressStage0;
static GT_U32 orig_numEntriesIngressStage1;


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xBB}                /* srcMac */
};

/* Unicast unknown da mac */
static TGF_MAC_ADDR     prvTgfUcastUnknownMac =
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA};

/* Unicast da mac */
static TGF_MAC_ADDR     prvTgfUcastKnownMac =
    {0x00, 0x00, 0x00, 0x44, 0x55, 0x44};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};


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
static TGF_PACKET_PART_STC prvTgfPacketTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of single tagged packet with CRC */
#define PRV_TGF_PACKET_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoTag =
{
    PRV_TGF_PACKET_TAG_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketTagPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfPolicerPortModeAddressSelectCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management and Policy Counters
*/
static GT_VOID prvTgfPolicerPortModeAddressSelectCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT policerStage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT mngSet
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrGreen;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrRed;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrYellow;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC  prvTgfMngCntrDrop;

    cpssOsMemSet(&prvTgfMngCntrGreen, 0, sizeof(prvTgfMngCntrGreen));
    cpssOsMemSet(&prvTgfMngCntrRed, 0, sizeof(prvTgfMngCntrRed));
    cpssOsMemSet(&prvTgfMngCntrYellow, 0, sizeof(prvTgfMngCntrYellow));
    cpssOsMemSet(&prvTgfMngCntrDrop, 0, sizeof(prvTgfMngCntrDrop));

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, policerStage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerPortModeAddressSelectCfg function
* @endinternal
*
* @brief   Set Address Select configuration.
*
* @param[in] type                     - Address Select type: Full or Compressed
* @param[in] stage                    - Policer  mode: Ingress0, Ingress1 or Egress.
* @param[in] portNumIdx               - Port index
* @param[in] stormType                - Storm ype
* @param[in] index                    -  for given storm type and port
* @param[in] entryPtr                 - pointer to metering entry 1
*
* @param[out] meterEntryIdxPtr         - index of the first metering entry
*                                       None
*/
static GT_VOID prvTgfPolicerPortModeAddressSelectCfg
(
    IN PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT   type,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT                   stage,
    IN GT_U8                                            portNumIdx,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT                   stormType,
    IN GT_U32                                           index,
    IN  PRV_TGF_POLICER_ENTRY_STC                       *entryPtr,
    OUT GT_U32                                          *meterEntryIdxPtr
)
{
    GT_STATUS                               rc;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT     tbParams;
    PRV_TGF_POLICER_MEMORY_CTRL_MODE_ENT    memMode;
    GT_U32                                   portNum;
    GT_U32                                   notIngressPort;


    portNum = prvTgfPortsArray[portNumIdx];
    notIngressPort = prvTgfPortsArray[(portNumIdx + 1)%prvTgfPortsNum];

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        portNum = notIngressPort;
    }

    /* store the default meterMode */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, stage,
                                        &defaultMeterMode);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d, %d, %d", prvTgfDevNum,
                                 stage, defaultMeterMode);

    /* Configure the given stage to work in Port mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                 stage, PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* Enable metering */
    rc = prvTgfPolicerMeteringEnableSet(stage, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d", prvTgfDevNum);

    /* Set metered Packet Size Mode as CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E */
    rc = prvTgfPolicerPacketSizeModeSet(stage, CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerPacketSizeModeSet: %d", stage);


    /* Disable counting */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,
                                      PRV_TGF_POLICER_COUNTING_DISABLE_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d",
                                 prvTgfDevNum, stage);

    /* Clear management counters */
    prvTgfPolicerPortModeAddressSelectCountersReset(stage, entryPtr->mngCounterSet);

    /* Enable port metering on the given port */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, stage, portNum, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d",
                                 prvTgfDevNum, stage, portNum);

    /* Configure for given stage the address select type */
    rc = prvTgfPolicerPortModeAddressSelectSet(prvTgfDevNum, stage, type);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortModeAddressSelectSet: %d, %d, %d",
                                 prvTgfDevNum, stage, type);


    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* configure memory mode */
        if (type == PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E)
        {
            if (stage == PRV_TGF_POLICER_STAGE_INGRESS_0_E)
            {
                /* Policer0 - 256 entries, Policer1 - 28 entries */
                memMode = PRV_TGF_POLICER_MEMORY_CTRL_MODE_3_E;
            }
            else
            {
                /* Policer1 - 256 entries, Policer0 - 28 entries */
                memMode = PRV_TGF_POLICER_MEMORY_CTRL_MODE_0_E;
            }
        }
        else
        if (type == PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_EXCLUSIVE_E)
        {
            if (stage == PRV_TGF_POLICER_STAGE_INGRESS_0_E)
            {
                /* Policer0 - all entries, Policer1 - no entries */
                memMode = PRV_TGF_POLICER_MEMORY_CTRL_MODE_1_E;
            }
            else
            {
                /* Policer1 - all entries, Policer0 - no entries */
                memMode = PRV_TGF_POLICER_MEMORY_CTRL_MODE_2_E;
            }
            type = PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E;
        }
        else
        {
            /* Address select type is compressed */
            if (stage == PRV_TGF_POLICER_STAGE_INGRESS_0_E)
            {
                /* Policer0 - 112 entries, Policer1 - 172 entries */
                memMode = PRV_TGF_POLICER_MEMORY_CTRL_MODE_4_E;
            }
            else
            {
                /* Policer1 - 112 entries, Policer0 - 172 entries */
                memMode = PRV_TGF_POLICER_MEMORY_CTRL_MODE_5_E;
            }
        }

        rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum, memMode,
                                        0,0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d", prvTgfDevNum,
                                     memMode);
    }


    /* Configure index for given storm type */
    rc = prvTgfPolicerPortStormTypeIndexSet(prvTgfDevNum, stage, portNum,
                                            stormType, index);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortStormTypeIndexSet: %d, %d, %d, %d",
                                 prvTgfDevNum, stage, portNum, stormType);

    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(prvTgfDevNum))
    {
        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            /* the multiport group devices use 6 bits of global port for index calculation */
            portNum = portNum % 64;
        }
        else
        {
            /* the multiport group devices use local port for index calculation */
            portNum = portNum % 16;
        }
    }

    /* calculate metering entry index */
    if (type == PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E)
    {
        /* Full mode */
        *meterEntryIdxPtr = ((64 * index) + portNum);
    }
    else
    {
         /* Compressed mode */
        *meterEntryIdxPtr = ((portNum * 4) + index);
    }

    /* Configure metering entry */
    rc = prvTgfPolicerEntrySet(stage, *meterEntryIdxPtr, entryPtr,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                 stage, *meterEntryIdxPtr);


    /* Configure FDB entry if needed, only two types are expected: uc and ucUnknown */
    if (stormType == PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E)
    {
        rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfUcastKnownMac,
                                              PRV_TGF_VLANID_2_CNS,
                                              prvTgfDevNum,
                                              notIngressPort,
                                              GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d, %d, %d",
                                     prvTgfUcastKnownMac, PRV_TGF_VLANID_2_CNS, notIngressPort);
    }
}

/**
* @internal prvTgfPolicerPortModeAddressSelectPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNumIdx               - port number index to send packet from
*                                       None
*/
static GT_VOID prvTgfPolicerPortModeAddressSelectPacketSend
(
    IN GT_U8    portNumIdx
)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet with single tag */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoTag, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* send Packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portNumIdx]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portNumIdx]);
}


/**
* @internal prvTgfPolicerPortModeAddressSelectManagementCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*/
static GT_VOID prvTgfPolicerPortModeAddressSelectManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       policerStage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT    mngCntrSet,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            mngCntrSet,
                                            mngCntrType,
                                            &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgf3PolicerManagementCountersGet: %d, ", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL4_STRING_MAC(
        duLowCntr, prvTgfMngCntr.duMngCntr.l[0],
        "prvTgfMngCntr.duMngCntr.l[0] = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        prvTgfMngCntr.duMngCntr.l[0], policerStage, mngCntrSet, mngCntrType);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        duHiCntr, prvTgfMngCntr.duMngCntr.l[1],
        "prvTgfMngCntr.duMngCntr.l[1] = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        prvTgfMngCntr.duMngCntr.l[1], policerStage, mngCntrSet, mngCntrType);

    UTF_VERIFY_EQUAL4_STRING_MAC(
        packetMngCntr, prvTgfMngCntr.packetMngCntr,
        "prvTgfMngCntr.packetMngCntr = %d, policerStage = %d, mngCntrSet = %d, mngCntrType = %d\n",
        prvTgfMngCntr.packetMngCntr, policerStage, mngCntrSet, mngCntrType);
}


/**
* @internal prvTgfPolicerPortModeAddressSelectCheckResults function
* @endinternal
*
* @brief   Check results
*
* @param[in] stage                    - Policer  mode: Ingress0, Ingress1 or Egress.
* @param[in] portNumIdx               - Port index
* @param[in] stormType                - Storm ype
* @param[in] entryPtr                 - pointer to metering entry 1
*                                       None
*/
static GT_VOID prvTgfPolicerPortModeAddressSelectCheckResults
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT                   stage,
    IN GT_U8                                            portNumIdx,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT                   stormType,
    IN  PRV_TGF_POLICER_ENTRY_STC                       *entryPtr
)
{
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          dropPktsNum;    /* number of dropped packets */
    GT_U32                          greenPktsNum;   /* number of green packets  */
    GT_U32                          yellowPktsNum;  /* number of yellow packets  */
    GT_U32                          redPktsNum;     /* number of red packets     */
    GT_U32                          expectedPktsNum; /* the expected packets number to receive */
    GT_U32                          expectedBytes;  /* expected bytes number     */
    GT_U32                          receivePortIdx;  /* index of receive port   */
    GT_STATUS                       rc;             /* return code              */

    dropPktsNum     = 0;
    greenPktsNum    = 0;
    yellowPktsNum   = 0;
    redPktsNum      = 0;

    receivePortIdx = ((portNumIdx + 1) % prvTgfPortsNum);

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

        if (portNumIdx == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        if (((stormType == PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E) &&
            (receivePortIdx == portIter)) ||
            (stormType == PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E))
        {

            /* Calculate green packets */
            if (entryPtr->tbParams.srTcmParams.cbs > 10240)
            {
                greenPktsNum = ((entryPtr->tbParams.srTcmParams.cbs - 10240) + 63) / 64;
            }

            /* Calculate yellow and dropped packets */
            if (entryPtr->tbParams.srTcmParams.ebs > 10240)
            {
                if (prvTgfBurstCount > greenPktsNum)
                {
                    yellowPktsNum = ((entryPtr->tbParams.srTcmParams.ebs - 10240) + 63) / 64;

                    if (prvTgfBurstCount < (yellowPktsNum + greenPktsNum))
                    {
                        yellowPktsNum = (prvTgfBurstCount - greenPktsNum);
                    }

                    if (entryPtr->yellowPcktCmd == PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E)
                    {
                        dropPktsNum += yellowPktsNum;
                        yellowPktsNum = 0;
                    }
                }
            }

            /* Calculate red and dropped packets */
            if (prvTgfBurstCount > (greenPktsNum + dropPktsNum + yellowPktsNum))
            {
                redPktsNum = (prvTgfBurstCount -
                              (greenPktsNum + dropPktsNum + yellowPktsNum));

                if (entryPtr->redPcktCmd == PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E)
                {
                    dropPktsNum += redPktsNum;
                    redPktsNum = 0;
                }
            }


            if (greenPktsNum > prvTgfBurstCount)
            {
                greenPktsNum = prvTgfBurstCount;
            }
            if (dropPktsNum > prvTgfBurstCount)
            {
                dropPktsNum = prvTgfBurstCount;
            }
            if (yellowPktsNum > prvTgfBurstCount)
            {
                yellowPktsNum = prvTgfBurstCount;
            }
            if (redPktsNum > prvTgfBurstCount)
            {
                redPktsNum = prvTgfBurstCount;
            }

            expectedPktsNum = prvTgfBurstCount - dropPktsNum;

            UTF_VERIFY_EQUAL0_STRING_MAC(expectedPktsNum, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* Check management counters */

    expectedBytes = (greenPktsNum * 64);
    prvTgfPolicerPortModeAddressSelectManagementCountersCheck(stage,
                                                              entryPtr->mngCounterSet,
                                                              PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                                                              expectedBytes, 0, greenPktsNum);

    expectedBytes = (yellowPktsNum * 64);
    prvTgfPolicerPortModeAddressSelectManagementCountersCheck(stage,
                                                              entryPtr->mngCounterSet,
                                                              PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                                                              expectedBytes, 0,
                                                              yellowPktsNum);

    expectedBytes = (redPktsNum * 64);
    prvTgfPolicerPortModeAddressSelectManagementCountersCheck(stage,
                                                              entryPtr->mngCounterSet,
                                                              PRV_TGF_POLICER_MNG_CNTR_RED_E,
                                                              expectedBytes, 0,
                                                              redPktsNum);

    expectedBytes = (dropPktsNum * 64);
    prvTgfPolicerPortModeAddressSelectManagementCountersCheck(stage,
                                                              entryPtr->mngCounterSet,
                                                              PRV_TGF_POLICER_MNG_CNTR_DROP_E,
                                                              expectedBytes, 0,
                                                              dropPktsNum);
}

/**
* @internal prvTgfPolicerPortModeAddressSelectTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check results
*
* @param[in] type                     - Address Select type: Full or Compressed
* @param[in] stage                    - Policer  mode: Ingress0, Ingress1 or Egress.
* @param[in] portNumIdx               - Port index
* @param[in] stormType                - Storm ype
* @param[in] index                    -  for given storm type and port
* @param[in] entryPtr                 - pointer to metering entry
*                                       None
*/
static GT_VOID prvTgfPolicerPortModeAddressSelectTrafficGenerate
(
    IN PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT   type,
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT                   stage,
    IN GT_U8                                            portNumIdx,
    IN PRV_TGF_POLICER_STORM_TYPE_ENT                   stormType,
    IN GT_U32                                           index,
    IN  PRV_TGF_POLICER_ENTRY_STC                       *entryPtr
)
{
    GT_STATUS                           rc;             /* returned status */
    GT_U32                              meterEntryIdx = 0;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    PRV_TGF_POLICER_ENTRY_STC           tmpMeterEntry;
    GT_U32                               portNum;


    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        portNum = prvTgfPortsArray[(portNumIdx + 1)%prvTgfPortsNum];
    }
    else
    {
        portNum = prvTgfPortsArray[portNumIdx];
    }

    /* Configure Policer */
    prvTgfPolicerPortModeAddressSelectCfg(type,
                                          stage,
                                          portNumIdx,
                                          stormType,
                                          index,
                                          entryPtr,
                                          &meterEntryIdx);

    /* Set destination address: unicast or unknown unicast */
    if (stormType == PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E)
    {
        /* Set destination addres to known unicast */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfUcastKnownMac,
                         sizeof(prvTgfPacketL2Part.daMac));
    }
    else
    {
        /* Set destination addres to unknown unicast */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfUcastUnknownMac,
                         sizeof(prvTgfPacketL2Part.daMac));
    }

    /* Send packet */
    prvTgfPolicerPortModeAddressSelectPacketSend(portNumIdx);

    /* give time for device/simulation for packet processing */
    cpssOsTimerWkAfter(10);


    /* check Counters */
    prvTgfPolicerPortModeAddressSelectCheckResults(stage,
                                                   portNumIdx,
                                                   stormType,
                                                   entryPtr);

    /* Restore Policer Configurations */

    /* Disable port metering on the given port */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum, stage,
                                            portNum,
                                            GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortMeteringEnableSet: %d", prvTgfDevNum,
                                 stage, portNum);


    /* Restore index to 0 for given storm type */
    rc = prvTgfPolicerPortStormTypeIndexSet(prvTgfDevNum, stage,
                                            portNum,
                                            stormType, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortStormTypeIndexSet: %d", prvTgfDevNum,
                                 stage, portNum);

    /* Restore metering entry */
    cpssOsMemSet(&tmpMeterEntry, 0, sizeof(tmpMeterEntry));
    tmpMeterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    tmpMeterEntry.yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    tmpMeterEntry.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    tmpMeterEntry.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    tmpMeterEntry.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    /* Set maximal values */
    tmpMeterEntry.tbParams.srTcmParams.cir = 0xFFFFFFFF;
    tmpMeterEntry.tbParams.srTcmParams.cbs = 0xFFFFFFFF;
    tmpMeterEntry.tbParams.srTcmParams.ebs = 0xFFFFFFFF;
    rc = prvTgfPolicerEntrySet(stage, meterEntryIdx, &tmpMeterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d", prvTgfDevNum,
                                 stage, meterEntryIdx);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfPolicerPortModeAddressSelectConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*/
GT_VOID prvTgfPolicerPortModeAddressSelectConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* 1. Create new vlan on ports (0,0), (0,8), (0,18), (0,23) */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);
}

/**
* @internal prvTgfPolicerPortModeAddressSelectFullRedCmdDropTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Full.
*         Configure Port mode.
*         Configure different Storm Type index for each port for unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two red packets and red command - drop.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two red packets and red
*         command - forward.
*         Generate traffic:
*         Send from each port 2 unicast packets:
*         macDa = 00:00:00:44:55:44
*         macSa = 00:00:00:00:11:BB
*         Success Criteria:
*         All packets are dropped.
*         Management counters of each port counted two drop packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectFullRedCmdDropTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U32      stormTypeIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {3, 2, 1, 0};
    GT_U8       portNumIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {0, 3, 1, 2};
    PRV_TGF_POLICER_STORM_TYPE_ENT  stormType = PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E;
    PRV_TGF_POLICER_ENTRY_STC       meterEntry;
    GT_U32      stormTypeIdx;


    /* there should be a match with this entry in full mode */
    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.mngCounterSet    = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    meterEntry.tbParams.srTcmParams.cir = 1;
    meterEntry.tbParams.srTcmParams.cbs = 1;
    meterEntry.tbParams.srTcmParams.ebs = 1; /* only red packets */;
    meterEntry.redPcktCmd       = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
    meterEntry.modifyDscp       = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;


    /* set the conformance level as red */
    prvTgfPolicerConformanceLevelForce(2);

    /* Store default memory mode */
    rc = prvTgfPolicerMemorySizeModeGet(prvTgfDevNum, &defaultMemMode,
        &orig_numEntriesIngressStage0,
        &orig_numEntriesIngressStage1,
        NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeGet: %d", prvTgfDevNum);

    for (ii = 0; ii < PRV_ADDR_SELECT_ITERAT_NUM_CNS; ii++)
    {
        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            stormTypeIdx = 0;
        }
        else
        {
            stormTypeIdx = stormTypeIdxArr[ii];
        }

        prvTgfPolicerPortModeAddressSelectTrafficGenerate(
                            PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E,
                            stage,
                            portNumIdxArr[ii],
                            stormType,
                            stormTypeIdx,
                            &meterEntry);
    }
}

/**
* @internal prvTgfPolicerPortModeAddressSelectFullRedCmdFrwdTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Full.
*         Configure Port mode.
*         Configure different Storm Type index for each port for unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two red packets and red command - forward.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two red packets and red
*         command - drop.
*         Generate traffic:
*         Send from each port 2 unicast packets:
*         macDa = 00:00:00:44:55:44
*         macSa = 00:00:00:00:11:BB
*         Success Criteria:
*         All packets received, each time on another port.
*         Management counters of each port counted two red packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectFullRedCmdFrwdTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U32      stormTypeIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {3, 2, 1, 0};
    GT_U8       portNumIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {2, 3, 0, 1};
    PRV_TGF_POLICER_STORM_TYPE_ENT  stormType = PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E;
    PRV_TGF_POLICER_ENTRY_STC       meterEntry;
    GT_U32      stormTypeIdx;

    /* there should be a match with this entry in full mode */
    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET2_E;
    meterEntry.tbParams.srTcmParams.cir = 1;
    meterEntry.tbParams.srTcmParams.cbs = 10240;
    meterEntry.tbParams.srTcmParams.ebs = 10240; /* only red packets */
    meterEntry.modifyDscp       = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    /* set the conformance level as red */
    prvTgfPolicerConformanceLevelForce(2);

    /* Store default memory mode */
    rc = prvTgfPolicerMemorySizeModeGet(prvTgfDevNum, &defaultMemMode,
        &orig_numEntriesIngressStage0,
        &orig_numEntriesIngressStage1,
        NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeGet: %d", prvTgfDevNum);

    for (ii = 0; ii < PRV_ADDR_SELECT_ITERAT_NUM_CNS; ii++)
    {
        if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            stormTypeIdx = 0;
        }
        else
        {
            stormTypeIdx = stormTypeIdxArr[ii];
        }

        prvTgfPolicerPortModeAddressSelectTrafficGenerate(
                            PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E,
                            stage,
                            portNumIdxArr[ii],
                            stormType,
                            stormTypeIdx,
                            &meterEntry);
    }
}

/**
* @internal prvTgfPolicerPortModeAddressSelectCompresYellowCmdDropTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Compressed.
*         Configure Port mode.
*         Configure different Storm Type index for each port for unkown unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two yellow packets and yellow command - drop.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - forward.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets are dropped.
*         Management counters of each port counted two drop packets.
*         Note: This test is not valid for Egress stage
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectCompresYellowCmdDropTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U32      stormTypeIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {3, 2, 1, 0};
    GT_U8       portNumIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {3, 2, 0, 1};
    PRV_TGF_POLICER_STORM_TYPE_ENT  stormType = PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E;
    PRV_TGF_POLICER_ENTRY_STC       meterEntry;

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        return;
    }

    /* there should be a match with this entry in full mode */
    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    meterEntry.tbParams.srTcmParams.cir = 1;
    meterEntry.tbParams.srTcmParams.cbs = 10240;
    meterEntry.tbParams.srTcmParams.ebs = 10368; /* two yellow packets */
    meterEntry.modifyDscp       = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    /* set the conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    /* Store default memory mode */
    rc = prvTgfPolicerMemorySizeModeGet(prvTgfDevNum, &defaultMemMode,
        &orig_numEntriesIngressStage0,
        &orig_numEntriesIngressStage1,
        NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeGet: %d", prvTgfDevNum);

    for (ii = 0; ii < PRV_ADDR_SELECT_ITERAT_NUM_CNS; ii++)
    {
        prvTgfPolicerPortModeAddressSelectTrafficGenerate(
                            PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E,
                            stage,
                            portNumIdxArr[ii],
                            stormType,
                            stormTypeIdxArr[ii],
                            &meterEntry);
    }
}

/**
* @internal prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwdTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Compressed.
*         Configure Port mode.
*         Configure different Storm Type index for each port for unkown unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two yellow packets and yellow command - forward.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - drop.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets should be received on all ports.
*         Management counters of each port counted two yellow packets.
*         Note: This test is not valid for Egress stage
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectCompresYellowCmdFrwdTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U32      stormTypeIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {3, 2, 1, 0};
    GT_U8       portNumIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {0, 3, 1, 2};
    PRV_TGF_POLICER_STORM_TYPE_ENT  stormType = PRV_TGF_POLICER_STORM_TYPE_UC_UNKNOWN_E;
    PRV_TGF_POLICER_ENTRY_STC       meterEntry;

    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        return;
    }

    /* there should be a match with this entry in full mode */
    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.yellowPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET1_E;
    meterEntry.tbParams.srTcmParams.cir = 1;
    meterEntry.tbParams.srTcmParams.cbs = 10240;
    meterEntry.tbParams.srTcmParams.ebs = 10368; /* two yellow packets */
    meterEntry.modifyDscp       = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    /* set the conformance level as yellow */
    prvTgfPolicerConformanceLevelForce(1);

    /* Store default memory mode */
    rc = prvTgfPolicerMemorySizeModeGet(prvTgfDevNum, &defaultMemMode,
        &orig_numEntriesIngressStage0,
        &orig_numEntriesIngressStage1,
        NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeGet: %d", prvTgfDevNum);

    for (ii = 0; ii < PRV_ADDR_SELECT_ITERAT_NUM_CNS; ii++)
    {
        prvTgfPolicerPortModeAddressSelectTrafficGenerate(
                            PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E,
                            stage,
                            portNumIdxArr[ii],
                            stormType,
                            stormTypeIdxArr[ii],
                            &meterEntry);
    }
}

/**
* @internal prvTgfPolicerPortModeAddressSelectFullGreenTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Full.
*         Configure Port mode.
*         Configure different Storm Type index for each port for known unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two green packets.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - forward.
*         This test should be good also for egress packets, thus the storm
*         indexes should be 0 or 1.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets are received.
*         Management counters of each port counted two green packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectFullGreenTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U8       portNumIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {3, 0, 2, 1};
    PRV_TGF_POLICER_STORM_TYPE_ENT  stormType = PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E;
    PRV_TGF_POLICER_ENTRY_STC       meterEntry;

    /* there should be a match with this entry in full mode */
    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET2_E;
    meterEntry.tbParams.srTcmParams.cir = 1;
    meterEntry.tbParams.srTcmParams.cbs = 10368; /* two green packets */
    meterEntry.tbParams.srTcmParams.ebs = 1;
    meterEntry.modifyDscp       = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    /* set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    /* Store default memory mode */
    rc = prvTgfPolicerMemorySizeModeGet(prvTgfDevNum, &defaultMemMode,
        &orig_numEntriesIngressStage0,
        &orig_numEntriesIngressStage1,
        NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeGet: %d", prvTgfDevNum);

    for (ii = 0; ii < PRV_ADDR_SELECT_ITERAT_NUM_CNS; ii++)
    {
        prvTgfPolicerPortModeAddressSelectTrafficGenerate(
                            PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E,
                            stage,
                            portNumIdxArr[ii],
                            stormType,
                            0,
                            &meterEntry);

        if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            /* Repeat the test in "exclusive" memory mode */
            prvTgfPolicerPortModeAddressSelectTrafficGenerate(
                                PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_EXCLUSIVE_E,
                                stage,
                                portNumIdxArr[ii],
                                stormType,
                                0,
                                &meterEntry);
        }
    }
}

/**
* @internal prvTgfPolicerPortModeAddressSelectCompresGreenTrafficGenerate function
* @endinternal
*
* @brief   Configure Address Select mode to Compressed.
*         Configure Port mode.
*         Configure different Storm Type index for each port for unkown unicast.
*         Configure for each port and storm type index matching meter entry,
*         with two green packets.
*         Configure for each port and storm type index not matching meter
*         entry (match for compressed mode), with two yellow packets and yellow
*         command - forward.
*         This test should be good also for egress packets, thus the storm
*         indexes should be 0 or 1.
*         Generate traffic:
*         Send from each port 2 unknown unicast packets:
*         macDa = 00:00:00:00:11:BB
*         macSa = 00:00:00:00:11:AA
*         Success Criteria:
*         All packets are received.
*         Management counters of each port counted two green packets.
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectCompresGreenTrafficGenerate
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U8       portNumIdxArr[PRV_ADDR_SELECT_ITERAT_NUM_CNS] = {3, 0, 2, 1};
    PRV_TGF_POLICER_STORM_TYPE_ENT  stormType = PRV_TGF_POLICER_STORM_TYPE_UC_KNOWN_E;
    PRV_TGF_POLICER_ENTRY_STC       meterEntry;

    /* there should be a match with this entry in full mode */
    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET0_E;
    meterEntry.tbParams.srTcmParams.cir = 1;
    meterEntry.tbParams.srTcmParams.cbs = 10368; /* two green packets */
    meterEntry.tbParams.srTcmParams.ebs = 10240;
    meterEntry.modifyDscp       = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyUp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    meterEntry.modifyDp         = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

    /* set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    /* Store default memory mode */
    rc = prvTgfPolicerMemorySizeModeGet(prvTgfDevNum, &defaultMemMode,
        &orig_numEntriesIngressStage0,
        &orig_numEntriesIngressStage1,
        NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeGet: %d", prvTgfDevNum);

    for (ii = 0; ii < PRV_ADDR_SELECT_ITERAT_NUM_CNS; ii++)
    {
        prvTgfPolicerPortModeAddressSelectTrafficGenerate(
                            PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E,
                            stage,
                            portNumIdxArr[ii],
                            stormType,
                            0,
                            &meterEntry);
    }
}


/**
* @internal prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerPortModeAddressSelectGlobalConfigurationRestore
(
    void
)
{
    GT_STATUS   rc = GT_OK;

    /****************/
    /* Tables Reset */
    /****************/

    /* set the conformance level as green */
    prvTgfPolicerConformanceLevelForce(0);

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);
}

/**
* @internal prvTgfPolicerPortModeAddressSelectConfigurationRestore function
* @endinternal
*
* @brief   Restore Policer per stage test configuration
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerPortModeAddressSelectConfigurationRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      ii;

    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum, defaultMemMode,
                                        orig_numEntriesIngressStage0,
                                        orig_numEntriesIngressStage1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d",
                                     prvTgfDevNum, defaultMemMode);
    }

    /* Restore meter mode */
    rc = prvTgfPolicerStageMeterModeSet(stage, defaultMeterMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                 stage, defaultMeterMode);

    /* Set metered Packet Size Mode as CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E */
    rc = prvTgfPolicerPacketSizeModeSet(stage, CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerPacketSizeModeSet: %d", stage);


    /* Restore counting to be Billing */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d",
                                 prvTgfDevNum, stage);

    /* Clear management counters */
    for (ii = 0; ii < PRV_TGF_POLICER_MNG_CNTR_SET2_E; ii++)
    {
        prvTgfPolicerPortModeAddressSelectCountersReset(stage, ii);
    }


    /* Configure the address select type to Full*/
    rc = prvTgfPolicerPortModeAddressSelectSet(prvTgfDevNum, stage,
                                               PRV_TGF_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerPortModeAddressSelectSet: %d, %d",
                                 prvTgfDevNum, stage);
}

