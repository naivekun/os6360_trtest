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
* @file prvTgfPolicerEarchMeterEntry.c
*
* @brief Tests for new & updted metering entry fields due to earch format.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <policer/prvTgfPolicerEarchMeterEntry.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>

#define PRV_TGF_VLANID_CNS 13

/* port index 0 */
#define PRV_TGF_PORT_IDX_0_CNS     0
/* port index 1 */
#define PRV_TGF_PORT_IDX_1_CNS     1
/* port index 2 */
#define PRV_TGF_PORT_IDX_2_CNS     2
/* port index 3 */
#define PRV_TGF_PORT_IDX_3_CNS     3

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/******************************* Test packet **********************************/

/* L2 tunnel part */
static TGF_PACKET_L2_STC prvTgfPacketL2TunnelPart = {
    {0x00, 0x00, 0x00, 0x00, 0x09, 0x0A},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* IPv4 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
                                              {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* IPv4 tunnel part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4TunnelPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0   ,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    { 1, 2, 3, 4},   /* srcAddr */
    { 5, 6, 7, 8}    /* dstAddr */
};

/* IPv4 passenfer part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4PassengerPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0   ,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,  /* csum */
    { 17,  34,  51,  68},   /* srcAddr */
    { 85, 102, 119, 136}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of IPv4 over IPv4 packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4OverIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,          &prvTgfPacketL2TunnelPart},/*type, partPtr*/
    {TGF_PACKET_PART_VLAN_TAG_E,    &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,   &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4TunnelPart},
    {TGF_PACKET_PART_IPV4_E,        &prvTgfPacketIpv4PassengerPart},
    {TGF_PACKET_PART_PAYLOAD_E,     &prvTgfPacketPayloadPart}
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

/* save for restore counting mode */
static PRV_TGF_POLICER_COUNTING_MODE_ENT
                    saveCountMode[PRV_TGF_POLICER_STAGE_NUM];

/* save for restore counting trigger by port status */
static GT_BOOL saveCountTriggerByPort[PRV_TGF_POLICER_STAGE_NUM];

/* saved policer configuration */
static PRV_TGF_POLICER_ENTRY_STC            saveMeterEntry;

/* save for restore MRU value */
static GT_U32 saveMru[PRV_TGF_POLICER_STAGE_NUM];
static GT_U32       currentMru = 0;
static GT_U32       currentPacketSize = 0;
static GT_U32       currentCbsMoreThanMru = 0;
static GT_U32       currentEbsMoreThanMru = 0;
static PRV_TGF_POLICER_METER_RESOLUTION_ENT currentMeterResolution = PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
static PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT currentFirstPacketColor = PRV_TGF_POLICER_MNG_CNTR_GREEN_E;
static GT_U32   currentActualExpectedPackets[PRV_TGF_POLICER_MNG_CNTR_DROP_E + 1] = {0,0,0,0};
static GT_U32   currentExpectDrop = 0;
/* names of the conformance levels -- for print info */
static GT_CHAR* conformanceLevelNameStrArr[] =
    {"green" , "yellow" , "red" ,"drop" };
static GT_CHAR* policerStageNameStrArr[] =
    {"iplr0" , "iplr1" , "eplr"};

CPSS_POLICER_PACKET_SIZE_MODE_ENT       packetSizeForRestore;

#define EXPECT_GREEN    PRV_TGF_POLICER_MNG_CNTR_GREEN_E
#define EXPECT_YELLOW   PRV_TGF_POLICER_MNG_CNTR_YELLOW_E
#define EXPECT_RED      PRV_TGF_POLICER_MNG_CNTR_RED_E
#define __EXPECT_DROP   PRV_TGF_POLICER_MNG_CNTR_DROP_E
#define PLUS_DROP       (1<<10)

static GT_BOOL  debug_enable = GT_FALSE;
static GT_U32 debug_policerUnit = 0;
static GT_U32 debug_sectionId = 0;

/* macro to check if need to run this phase */
#define ALLOW_RUN_PHASE_MAC(phaseId)    \
    ((debug_enable == GT_FALSE || (phaseId) == debug_sectionId) ? 1 : 0)

/* macro to check if need to run this policer unit */
#define ALLOW_RUN_STAGE_MAC(policerUnit) \
    ((debug_enable == GT_FALSE || (policerUnit) == debug_policerUnit) ? 1 : 0)


/* get 'normilized' bucket size */
static GT_U32   getBucketSize(
    IN GT_U32    diffBucketFromMru_perSinglePacket ,
    IN PRV_TGF_POLICER_METER_RESOLUTION_ENT meterResolution
)
{
    GT_U32  value = 0;

    currentMeterResolution = meterResolution;
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        if(currentCbsMoreThanMru == 0 && currentEbsMoreThanMru == 0)
        {
            currentFirstPacketColor = PRV_TGF_POLICER_MNG_CNTR_RED_E;
        }
        else
        if(currentCbsMoreThanMru == 0)
        {
            currentFirstPacketColor = PRV_TGF_POLICER_MNG_CNTR_YELLOW_E;
        }
        else
        {
            currentFirstPacketColor = PRV_TGF_POLICER_MNG_CNTR_GREEN_E;
        }
    }
    else
    {
        /* this is non sip5_15 or 'Packet based Meter' resolution */
        return diffBucketFromMru_perSinglePacket;
    }
    return (currentMru + diffBucketFromMru_perSinglePacket /*for first packet */) +
            value; /*for rest of the packets */
}

/* get 'normilized' CBS */
static GT_U32   getCbs(
    IN GT_U32    diffCbsFromMru_perSinglePacket ,
    IN PRV_TGF_POLICER_METER_RESOLUTION_ENT meterResolution
)
{
    currentCbsMoreThanMru = (diffCbsFromMru_perSinglePacket)?1:0;
    return getBucketSize(diffCbsFromMru_perSinglePacket,meterResolution);
}
/* get 'normilized' EBS */
static GT_U32   getEbs(
    IN GT_U32    diffEbsFromMru_perSinglePacket ,
    IN PRV_TGF_POLICER_METER_RESOLUTION_ENT meterResolution
)
{
    currentEbsMoreThanMru = (diffEbsFromMru_perSinglePacket)?1:0;
    return getBucketSize(diffEbsFromMru_perSinglePacket,meterResolution);
}

/* for sip 5_15 burst count differ between the deferent test cases */
static void sip5_15_setBurstCount(IN GT_U32 expectedColorForTraffic)
{
    if(expectedColorForTraffic & PLUS_DROP)/*check flag*/
    {
        currentExpectDrop = 1;
    }
    else
    {
        currentExpectDrop = 0;
    }

    expectedColorForTraffic &= ~PLUS_DROP;


    currentActualExpectedPackets[EXPECT_GREEN] =
    currentActualExpectedPackets[EXPECT_YELLOW] =
    currentActualExpectedPackets[EXPECT_RED] = 0;

    currentActualExpectedPackets[__EXPECT_DROP] = currentExpectDrop;

    if(expectedColorForTraffic == EXPECT_GREEN)
    {
        /* first is green */
        currentActualExpectedPackets[EXPECT_GREEN] = currentExpectDrop ? 0 : 1;

        PRV_UTF_LOG1_MAC("test expect green %s, also actual send 1 packet \n" ,
            currentExpectDrop ? "(to be dropped)" : "");
    }
    else
    if(expectedColorForTraffic == EXPECT_YELLOW)
    {
        currentActualExpectedPackets[EXPECT_YELLOW] = currentExpectDrop ? 0 : 1;

        if(currentFirstPacketColor == PRV_TGF_POLICER_MNG_CNTR_GREEN_E)
        {
            /* first green , second yellow */
            currentActualExpectedPackets[EXPECT_GREEN] = 1;

            PRV_UTF_LOG1_MAC("test expect yellow %s, but actual send 2 packet : first green , second yellow \n" ,
                currentExpectDrop ? "(to be dropped)" : "");
        }
        else
        {
            /* first is yellow */

            PRV_UTF_LOG1_MAC("test expect yellow %s, also actual send 1 packet \n",
                currentExpectDrop ? "(to be dropped)" : "");
        }
    }
    else /* red */
    {
        currentActualExpectedPackets[EXPECT_RED] = currentExpectDrop ? 0 : 1;

        if(currentFirstPacketColor == PRV_TGF_POLICER_MNG_CNTR_GREEN_E)
        {
            /* test expect byte count more then CBS and EBS .
                so since first packet is green (because CBS,EBS more then MRU),
                third packet is Green
                the device will modify entry after first packet to make next packet as yellow
                (because CBS less then MRU but EBS still more then MRU)
                second packet is Yellow
                the device will modify entry after second packet to make next packet as red
                (because CBS,EBS less then MRU)

                third packet is RED
            */

            /* first green , second yellow , third red */
            currentActualExpectedPackets[EXPECT_GREEN] = 1;
            currentActualExpectedPackets[EXPECT_YELLOW] = 1;
            PRV_UTF_LOG1_MAC("test expect red %s, but actual send 3 packet : first green , second yellow , third red \n",
                currentExpectDrop ? "(to be dropped)" : "");
        }
        else
        if(currentFirstPacketColor == PRV_TGF_POLICER_MNG_CNTR_YELLOW_E)
        {
            /* first yellow , second red */
            currentActualExpectedPackets[EXPECT_YELLOW] = 1;
            PRV_UTF_LOG1_MAC("test expect red %s, but actual send 2 packet : first yellow , second red \n",
                currentExpectDrop ? "(to be dropped)" : "");
        }
        else
        {
            /* first is red */
            PRV_UTF_LOG1_MAC("test expect red %s, also actual send 1 packet \n",
                currentExpectDrop ? "(to be dropped)" : "");
        }
    }

    prvTgfBurstCount = currentActualExpectedPackets[EXPECT_GREEN]  +
                       currentActualExpectedPackets[EXPECT_YELLOW] +
                       currentActualExpectedPackets[EXPECT_RED] +
                       currentActualExpectedPackets[__EXPECT_DROP];
}

/**
* @internal mruSet function
* @endinternal
*
* @brief   set needed MRU for the test .
*
* @param[in] stage                    - policer  (iplr0/1/eplr)
* @param[in] mru                      - policer MRU
*                                       None
*/
static void mruSet(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    IN GT_U32  mru
)
{
    GT_STATUS rc;

    currentMru = mru;
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: statistical metering :

            NOTE: we can not get yellow/red if currentMru will be 0 !!!

           to differentiate between 'with dsa' and 'without dsa' counting
           we need more then single packet

           first packet will be green on both cases , but second pocket will be :
           1. green  - for 'without dsa' mode
           2. yellow - for 'with dsa'


           expected behavior:
           scenario for 2 packets:
           MRU = 0x40
           CBS = 0x41 + 0x40
           EBS = 0x51 + 0x40

           ==================
           'without dsa' counting mode :
                   ==================
                   cpu set the entry:
                   CBS,EBS

                   (the pp) update signs :
                       sign0 = true , sign1 = true : because TB0 and TB1 more then MRU
                       (meaning next packet considered green)
                   ==================
                   first packet processed:
                        1. (the pp) consider as green because sign0 = true
                        2. (the pp) update token buckets:
                            a. TB0 = 0x41 + 0x40 - 0x40 = 0x41
                            b. TB1 = no change = 0x51 + 0x40
                        3. (the pp) update signs :
                            sign0 = true , sign1 = true : because TB0 and TB1 more then MRU
                            (meaning next packet considered green)
                   ==================
                   second packet processed:
                        1. (the pp) consider as green because sign0 = true
                        2. (the pp) update token buckets:
                            a. TB0 = 0x41 - 0x40 = 0x1
                            b. TB1 = no change = 0x51 + 0x40
                        3. (the pp) update signs :
                            sign0 = false , sign1 = true : because TB0 and TB1 more then MRU
                            (meaning next packet considered yellow)

           ==================
           'with dsa' counting mode :
                   ==================
same as 'without' : cpu set the entry:
same as 'without' : CBS,EBS

same as 'without' : (the pp) update signs :
same as 'without' :     sign0 = true , sign1 = true : because TB0 and TB1 more then MRU
same as 'without' :     (meaning next packet considered green)
                   ==================
                   first packet processed:
same as 'without' :     1. (the pp) consider as green because sign0 = true
                        2. (the pp) update token buckets:
!!!diff!!!                  a. TB0 = 0x41 + 0x40 - 0x50 = 0x31
                            b. TB1 = no change = 0x51 + 0x40 = 0x91
                        3. (the pp) update signs :
!!!diff!!!                  sign0 = false , sign1 = true : because TB0 lower then MRU and TB1 more then MRU
!!!diff!!!                  (meaning next packet considered yellow)
                   ==================
                   second packet processed:
!!!diff!!!              1. (the pp) consider as yellow because sign0 = false
                        2. (the pp) update token buckets:
!!!diff!!!                  a. TB0 = no change = 0x31
!!!diff!!!                  b. TB1 = 0x91 - 0x40 = 0x51
                        3. (the pp) update signs :
!!!diff!!!                  sign0 = false , sign1 = true : because TB0 lower then MRU and TB1 more then MRU
                            (meaning next packet considered yellow)

        */
        GT_U32  packetSize;
        GT_U32  partsCount = sizeof(prvTgfPacketIpv4OverIpv4PartArray) /
                     sizeof(prvTgfPacketIpv4OverIpv4PartArray[0]);

        rc = prvTgfPacketSizeGet(prvTgfPacketIpv4OverIpv4PartArray, partsCount,
                                                                       &packetSize);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "IPv4oIPv4, prvTgfPacketSizeGet: %d, %d",
                                     partsCount, packetSize);

        /*
            since CPU sets entry with initial values of MRU < packetSize <= CBS <= EBS

            when test want 'red' we will get :
            1. one green
            2. one yellow
            3. one red

            when test want 'yellow' we will get :
            1. two greens
            2. one yellow

            when test want 'green' we will get :
            1. three greens

            when test want 'drop' we will get :
            1. one green
            2. one yellow
            3. one drop <--- (the 'red' that was dropped)
        */
        currentMru = mru ? mru : 1;

        currentPacketSize = packetSize;
    }


    /* AUTODOC: set MRU value */
    rc = prvTgfPolicerMruSet(stage, currentMru);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruSet: %d, %d, %d",
                                 prvTgfDevNum, stage, currentMru);


    return;
}

/**
* @internal prvTgfPolicerEarchMeterEntryTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] portIndex                - port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPolicerEarchMeterEntryTestPacketSend
(
    IN GT_U32           portIndex,
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      portIter;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0,
                                                                          NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[portIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[portIndex]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPolicerEarchMeterEntryTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters according to expected value
*
* @param[in] sendPortIndex            -  sending port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPolicerEarchMeterEntryTestCheckCounters
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           expectedValue,
    IN GT_U32           callIdentifier
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    if(currentExpectDrop &&
       prvTgfBurstCount > 1 &&
       PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        /* we expect drops , but only last packet is dropped */
        expectedValue += (prvTgfBurstCount - 1);
    }


    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
                                       GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (sendPortIndex == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL4_STRING_MAC(prvTgfBurstCount,
                                   portCntrs.goodPktsRcv.l[0],
          "Rx another goodPktsRcv counter %d, than expected %d, on port %d, %d",
                                   portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                                   prvTgfPortsArray[portIter], callIdentifier);
        }
        else
        {
            UTF_VERIFY_EQUAL4_STRING_MAC(expectedValue,
                                    portCntrs.goodPktsSent.l[0],
         "Tx another goodPktsSent counter %d, than expected %d, on port %d, %d",
                                    portCntrs.goodPktsSent.l[0], expectedValue,
                                    prvTgfPortsArray[portIter], callIdentifier);
        }
    }
}

/**
* @internal prvTgfPolicerEarchEgrMeterEntryTestCheckCounters function
* @endinternal
*
* @brief   Check Eth counters on egress port according to expected value
*
* @param[in] receivePortIndex         - receiving port index
* @param[in] sendPortIndex            - sending port index
* @param[in] expectedValue            - expected number of transmitted packets
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPolicerEarchEgrMeterEntryTestCheckCounters
(
    IN GT_U32           receivePortIndex,
    IN GT_U32           sendPortIndex,
    IN GT_U32           expectedValue,
    IN GT_U32           callIdentifier
)
{
    GT_STATUS                       rc;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* check counters */
    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[receivePortIndex],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[receivePortIndex],
                                 callIdentifier);
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[sendPortIndex],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[sendPortIndex],
                                 callIdentifier);

    /* check Rx counters */
    UTF_VERIFY_EQUAL4_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
          "Rx another goodPktsRcv counter %d, than expected %d, on port %d, %d",
                               portCntrs.goodPktsRcv.l[0], prvTgfBurstCount,
                               prvTgfPortsArray[sendPortIndex], callIdentifier);
    /* check Tx counters */
    UTF_VERIFY_EQUAL4_STRING_MAC(expectedValue, portCntrs.goodPktsSent.l[0],
         "Tx another goodPktsSent counter %d, than expected %d, on port %d, %d",
                            portCntrs.goodPktsSent.l[0], expectedValue,
                            prvTgfPortsArray[receivePortIndex], callIdentifier);
}
/**
* @internal prvTgfPolicerEarchMeterEntryClearBillingCounters function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*
* @param[in] stage                    - policer stage
* @param[in] index                    - billing counter entry index
*                                       None
*/
static GT_VOID prvTgfPolicerEarchMeterEntryClearBillingCounters
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT   stage,
    IN GT_U32                           index
)
{
    PRV_TGF_POLICER_BILLING_ENTRY_STC   billingCntr;
    GT_STATUS                           rc;

    /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
    prvTgfPolicerCountingWriteBackCacheFlush(stage);

    cpssOsMemSet(&billingCntr, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));

    billingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_1_BYTE_E;
    billingCntr.greenCntr.l[0] = 0;
    billingCntr.greenCntr.l[1] = 0;
    billingCntr.yellowCntr.l[0] = 0;
    billingCntr.yellowCntr.l[1] = 0;
    billingCntr.redCntr.l[0] = 0;
    billingCntr.redCntr.l[1] = 0;
    billingCntr.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;

    rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum, stage, index, &billingCntr);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerBillingEntrySet: %d, %d, %d",
                                 prvTgfDevNum, stage, index);
}

/**
* @internal prvTgfPolicerEarchMeterEntryBillingCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Billing Counters
*
* @param[in] stage                    - policer stage
* @param[in] index                    - billing counter entry index
* @param[in] billingCntr              - expected counters value
*                                       None
*/
static GT_VOID prvTgfPolicerEarchMeterEntryBillingCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT        stage,
    IN GT_U32                                index,
    IN PRV_TGF_POLICER_BILLING_ENTRY_STC     billingCntr,
    IN GT_U32                                callIndex
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_BILLING_ENTRY_STC     readBillingCntr;

    /* read Billing Entry */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum,
                                      stage,
                                      index,
                                      GT_TRUE,
                                      &readBillingCntr);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                              "%d: cpssDxCh3PolicerBillingEntryGet: %d, %d, %d",
                                 callIndex, prvTgfDevNum, stage, index);

    /* check counters */
    UTF_VERIFY_EQUAL5_STRING_MAC(
        billingCntr.greenCntr.l[0], readBillingCntr.greenCntr.l[0],
        "%d: billingCntr.greenCntr.l[0] = %d, "
             "readBillingCntr.greenCntr.l[0] = %d, stage = %s, index = %d\n",
        callIndex, billingCntr.greenCntr.l[0], readBillingCntr.greenCntr.l[0],
        policerStageNameStrArr[stage], index);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        billingCntr.greenCntr.l[1], readBillingCntr.greenCntr.l[1],
        "%d: billingCntr.greenCntr.l[1] = %d, "
             "readBillingCntr.greenCntr.l[1] = %d, stage = %s, index = %d\n",
        callIndex, billingCntr.greenCntr.l[1], readBillingCntr.greenCntr.l[1],
        policerStageNameStrArr[stage], index);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        billingCntr.yellowCntr.l[0], readBillingCntr.yellowCntr.l[0],
        "%d: billingCntr.yellowCntr.l[0] = %d, "
             "readBillingCntr.yellowCntr.l[0] = %d, stage = %s, index = %d\n",
        callIndex, billingCntr.yellowCntr.l[0], readBillingCntr.yellowCntr.l[0],
        policerStageNameStrArr[stage], index);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        billingCntr.yellowCntr.l[1], readBillingCntr.yellowCntr.l[1],
        "%d: billingCntr.yellowCntr.l[1] = %d, "
             "readBillingCntr.yellowCntr.l[1] = %d, stage = %s, index = %d\n",
        callIndex, billingCntr.yellowCntr.l[1], readBillingCntr.yellowCntr.l[1],
        policerStageNameStrArr[stage], index);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        billingCntr.redCntr.l[0], readBillingCntr.redCntr.l[0],
        "%d: billingCntr.redCntr.l[0] = %d, "
             "readBillingCntr.redCntr.l[0] = %d, stage = %s, index = %d\n",
        callIndex, billingCntr.redCntr.l[0], readBillingCntr.redCntr.l[0],
        policerStageNameStrArr[stage], index);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        billingCntr.redCntr.l[1], readBillingCntr.redCntr.l[1],
        "%d: billingCntr.redCntr.l[1] = %d, "
             "readBillingCntr.redCntr.l[1] = %d, stage = %s, index = %d\n",
        callIndex, billingCntr.redCntr.l[1], readBillingCntr.redCntr.l[1],
        policerStageNameStrArr[stage], index);
}

/**
* @internal prvTgfPolicerEarchMeterEntryManagementCountersReset function
* @endinternal
*
* @brief   Resets the value of specified Management Counters
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
*                                       None
*/
static GT_VOID prvTgfPolicerEarchMeterEntryManagementCountersReset
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
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

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_GREEN_E, &prvTgfMngCntrGreen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_YELLOW_E, &prvTgfMngCntrYellow);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_RED_E, &prvTgfMngCntrRed);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);

    rc = prvTgfPolicerManagementCountersSet(prvTgfDevNum, stage,
        mngSet, PRV_TGF_POLICER_MNG_CNTR_DROP_E, &prvTgfMngCntrDrop);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvTgfPolicerManagementCountersSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerEarchMeterEntryManagementCountersCheck function
* @endinternal
*
* @brief   Check the value of specified Management Counters
*/
static GT_VOID prvTgfPolicerEarchMeterEntryManagementCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       policerStage,
    IN  PRV_TGF_POLICER_MNG_CNTR_SET_ENT    mngCntrSet,
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType,
    IN GT_U32 duLowCntr,
    IN GT_U32 duHiCntr,
    IN GT_U32 packetMngCntr,
    IN GT_U32 callIndex
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;

    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum, policerStage,
                                            mngCntrSet,
                                            mngCntrType,
                                            &prvTgfMngCntr);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "%d: prvTgf3PolicerManagementCountersGet: %d", callIndex, prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL5_STRING_MAC(
        duLowCntr, prvTgfMngCntr.duMngCntr.l[0],
        "%d: prvTgfMngCntr.duMngCntr.l[0] = %d, policerStage = %s, "
             "mngCntrSet = %d, mngCntrType = %s \n",
        callIndex, prvTgfMngCntr.duMngCntr.l[0], policerStageNameStrArr[policerStage], mngCntrSet,
        conformanceLevelNameStrArr[mngCntrType]);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        duHiCntr, prvTgfMngCntr.duMngCntr.l[1],
        "%d: prvTgfMngCntr.duMngCntr.l[1] = %d, policerStage = %s, "
             "mngCntrSet = %d, mngCntrType = %s \n",
        callIndex, prvTgfMngCntr.duMngCntr.l[1], policerStageNameStrArr[policerStage], mngCntrSet,
        conformanceLevelNameStrArr[mngCntrType]);

    UTF_VERIFY_EQUAL5_STRING_MAC(
        packetMngCntr, prvTgfMngCntr.packetMngCntr,
        "%d: prvTgfMngCntr.packetMngCntr = %d, policerStage = %s, "
             "mngCntrSet = %d, mngCntrType = %s \n",
        callIndex, prvTgfMngCntr.packetMngCntr, policerStageNameStrArr[policerStage], mngCntrSet,
        conformanceLevelNameStrArr[mngCntrType]);
}

/**
* @internal prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear function
* @endinternal
*
* @brief   Clear management counter set and billing entry
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
* @param[in] index                    - billing entry index
*                                       None
*/
static GT_VOID prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT   stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT mngSet,
    IN GT_U32                           index
)
{
    prvTgfPolicerEarchMeterEntryManagementCountersReset(stage, mngSet);
    prvTgfPolicerEarchMeterEntryClearBillingCounters(stage, index);
}

/**
* @internal prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck function
* @endinternal
*
* @brief   Check Management and Billing counters value
*
* @param[in] stage                    - policer stage
* @param[in] mngSet                   - management counter set
* @param[in] expMngCounters[4]        - expected Management counters value
* @param[in] index                    - billing entry index
* @param[in] countEntry               - expaected billing counters value
* @param[in] callIndex                - reference to this check sequence call
*                                       None
*/
static GT_VOID prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT       stage,
    IN PRV_TGF_POLICER_MNG_CNTR_SET_ENT     mngSet,
    IN GT_U32                               expMngCounters[4][3],
    IN GT_U32                               index,
    IN PRV_TGF_POLICER_BILLING_ENTRY_STC    countEntry,
    IN GT_U32                               callIndex
)
{
    GT_U32  ii;
    GT_U32  value;
    GT_U32  greenDropCount,yellowDropCount,redDropCount;

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
    {
        /* see logic that may send more then single packet ...
           but the test written in logic of single packet results */
        for(ii = 0; ii < 3 ; ii++)
        {

            /* ASSUMPTION : only one of the next : green/yellow/red/drop hold
               value different then ZERO */
            value = expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][ii]  +
                    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][ii] +
                    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][ii]    +
                    expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][ii]   ;


            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][ii]    = value * currentActualExpectedPackets[EXPECT_GREEN];
            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][ii]   = value * currentActualExpectedPackets[EXPECT_YELLOW];
            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][ii]      = value * currentActualExpectedPackets[EXPECT_RED];
            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][ii]     = value * currentActualExpectedPackets[__EXPECT_DROP];
        }

        /* ASSUMPTION : only one of the next : green/yellow/red/drop hold
           value different then ZERO */
        value = countEntry.greenCntr.l[0]  +
                countEntry.yellowCntr.l[0] +
                countEntry.redCntr.l[0]    ;

        greenDropCount = yellowDropCount = redDropCount = 0;
        if(currentActualExpectedPackets[__EXPECT_DROP])
        {
            if(countEntry.redCntr.l[0])
            {
                redDropCount = value * currentActualExpectedPackets[__EXPECT_DROP];
            }
            else
            if(countEntry.yellowCntr.l[0])
            {
                yellowDropCount = value * currentActualExpectedPackets[__EXPECT_DROP];
            }
            else
            {
                greenDropCount = value * currentActualExpectedPackets[__EXPECT_DROP];
            }
        }

        countEntry.greenCntr.l[0]   = greenDropCount  + (value * currentActualExpectedPackets[EXPECT_GREEN]);
        countEntry.yellowCntr.l[0]  = yellowDropCount + (value * currentActualExpectedPackets[EXPECT_YELLOW]);
        countEntry.redCntr.l[0]     = redDropCount    + (value * currentActualExpectedPackets[EXPECT_RED]);
    }


    prvTgfPolicerEarchMeterEntryManagementCountersCheck(stage, mngSet,
                            PRV_TGF_POLICER_MNG_CNTR_GREEN_E,
                            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][0],
                            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][1],
                            expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][2],
                            callIndex);

    prvTgfPolicerEarchMeterEntryManagementCountersCheck(stage, mngSet,
                           PRV_TGF_POLICER_MNG_CNTR_YELLOW_E,
                           expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][0],
                           expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][1],
                           expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][2],
                           callIndex);

    prvTgfPolicerEarchMeterEntryManagementCountersCheck(stage, mngSet,
                              PRV_TGF_POLICER_MNG_CNTR_RED_E,
                              expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0],
                              expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1],
                              expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2],
                              callIndex);

    prvTgfPolicerEarchMeterEntryManagementCountersCheck(stage, mngSet,
                             PRV_TGF_POLICER_MNG_CNTR_DROP_E,
                             expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][0],
                             expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][1],
                             expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][2],
                             callIndex);

    prvTgfPolicerEarchMeterEntryBillingCountersCheck(stage,
                                                     index,
                                                     countEntry,
                                                     callIndex);
}

/**
* @internal prvTgfPolicerEarchEgrMeterEntryManagementCountCheck function
* @endinternal
*
* @brief   Check Management counters value on egress port
*
* @param[in] mngCntrType              - packet type
*                                       None
*/
GT_VOID prvTgfPolicerEarchEgrMeterEntryManagementCountCheck
(
    IN  PRV_TGF_POLICER_MNG_CNTR_TYPE_ENT   mngCntrType
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_POLICER_MNG_CNTR_ENTRY_STC prvTgfMngCntr;
    /* get Management Counters */
    rc = prvTgfPolicerManagementCountersGet(prvTgfDevNum,
                                            PRV_TGF_POLICER_STAGE_EGRESS_E,
                                            PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                            mngCntrType,
                                            &prvTgfMngCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                       "prvTgf3PolicerManagementCountersGet: %d", prvTgfDevNum);

    /* check counters */
    UTF_VERIFY_EQUAL3_STRING_MAC(1, prvTgfMngCntr.packetMngCntr,
        "prvTgfMngCntr.packetMngCntr = %d, prvTgfMngCntr.duMngCntr.l[0] = %d, "
         "mngCntrType = %s \n",
        prvTgfMngCntr.packetMngCntr, prvTgfMngCntr.duMngCntr.l[0], conformanceLevelNameStrArr[mngCntrType]);
}

/**
* @internal prvTgfPolicerBasicEarchMeterEntryInitEntrySet function
* @endinternal
*
* @brief   Initial metering entry setting
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerBasicEarchMeterEntryInitEntrySet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS                             rc;
    PRV_TGF_POLICER_ENTRY_STC           meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U16                                 portNum;

    portNum = PRV_TGF_PORT_IDX_1_CNS;

    /* set policer to egress port if applicable */
    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        portNum = PRV_TGF_PORT_IDX_3_CNS;
    }

    currentMru = 0;

    /* Save Policer entry, configure counter set 1 */
    rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                stage,
                                prvTgfPortsArray[portNum],
                                &saveMeterEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfPolicerEntryGet: %d", prvTgfDevNum);

    cpssOsMemSet(&meterEntry, 0, sizeof(meterEntry));
    meterEntry.policerEnable = GT_TRUE;

    meterEntry.counterEnable = GT_TRUE;
    meterEntry.mngCounterSet = PRV_TGF_POLICER_MNG_CNTR_SET1_E;
    meterEntry.countingEntryIndex = prvTgfPortsArray[portNum];
    meterEntry.tbParams.srTcmParams.cir = 0;
    meterEntry.tbParams.srTcmParams.cbs = getCbs(1,meterEntry.byteOrPacketCountingMode);
    meterEntry.tbParams.srTcmParams.ebs = getEbs(1,meterEntry.byteOrPacketCountingMode);
    /* AUTODOC: Metering according to L2 */
    meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        meterEntry.modifyDscp = PRV_TGF_POLICER_MODIFY_DSCP_DISABLE_E;
        meterEntry.modifyUp   = PRV_TGF_POLICER_MODIFY_UP_DISABLE_E;
        meterEntry.modifyDp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        meterEntry.modifyExp  = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        meterEntry.modifyTc   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    }
    /* AUTODOC: Setting the initial metering entry */
    rc = prvTgfPolicerEntrySet(stage,
                               prvTgfPortsArray[portNum],
                               &meterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                 stage,
                                 prvTgfPortsArray[portNum]);
}

/**
* @internal prvTgfPolicerBasicEarchMeterEntryConfigurationSet function
* @endinternal
*
* @brief   Basic eArch metering entry test configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerBasicEarchMeterEntryConfigurationSet
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC        prvTgfMacEntry;
    GT_U16                             portNum;

    portNum = PRV_TGF_PORT_IDX_1_CNS;

    /* set policer to egress port if applicable */
    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        portNum = PRV_TGF_PORT_IDX_3_CNS;
        cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

        /* AUTODOC: Set FDB entry with local ePort */
        prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
        prvTgfMacEntry.isStatic = GT_TRUE;
        prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
        prvTgfMacEntry.dstInterface.devPort.portNum = portNum;
        cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther,
                          prvTgfPacketL2TunnelPart.daMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");
    }


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

    /* AUTODOC: Save port metering enable for restore */
    rc = prvTgfPolicerPortMeteringEnableGet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[portNum],
                                            &savePortMeterEnable[stage]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableGet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[portNum],
                                 savePortMeterEnable[stage]);

    /* AUTODOC: Enable port for metering */
    rc = prvTgfPolicerPortMeteringEnableSet(prvTgfDevNum,
                                            stage,
                                            prvTgfPortsArray[portNum],
                                            GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[portNum],
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
                                 "prvTgfPolicerStageMeterModeSet: %d, %d",
                                 stage,
                                 PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);

    /* AUTODOC: Save counting mode for restore */
    rc = prvTgfPolicerCountingModeGet(prvTgfDevNum,
                                      stage,
                                      &saveCountMode[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerCountingModeGet: %d, %d, %d",
                                 prvTgfDevNum, stage, saveCountMode[stage]);

    /* AUTODOC: Configure counting mode to billing */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerCountingModeSet: %d, %d %d",
                                 prvTgfDevNum, stage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    /* AUTODOC: Save counting trigger by port for restore */
    rc = prvTgfPolicerCountingTriggerByPortEnableGet(prvTgfDevNum, stage,
                                                &saveCountTriggerByPort[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                      "prvTgfPolicerCountingTriggerByPortEnableGet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveCountTriggerByPort[stage]);

    /* AUTODOC: Enable counting trigger by port */
    rc = prvTgfPolicerCountingTriggerByPortEnableSet(prvTgfDevNum, stage,
                                                     GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                      "prvTgfPolicerCountingTriggerByPortEnableSet: %d, %d, %d",
                                 prvTgfDevNum, stage, GT_TRUE);

    /* AUTODOC: Initial metering entry setting */
    prvTgfPolicerBasicEarchMeterEntryInitEntrySet(stage);

    /* AUTODOC: Save MRU for restore */
    rc = prvTgfPolicerMruGet(prvTgfDevNum, stage, &saveMru[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruGet: %d, %d, %d",
                                 prvTgfDevNum, stage, saveMru[stage]);

    /* AUTODOC: Save packetSize for restore */
    rc = prvTgfPolicerPacketSizeModeGet (prvTgfDevNum, stage,
                                         &packetSizeForRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                               "prvTgfPolicerPacketSizeModeGet: %d", stage);

}

/**
* @internal prvTgfPolicerBasicEarchMeterEntryRestore function
* @endinternal
*
* @brief   Basic eArch metering entry test restore configurations
*
* @param[in] stage                    - Policer stage
*                                       None
*/
static GT_VOID prvTgfPolicerBasicEarchMeterEntryRestore
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS                           rc;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U16                              portNum;

    portNum = PRV_TGF_PORT_IDX_1_CNS;

    /* set policer to egress port if applicable */
    if (PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
    {
        portNum = PRV_TGF_PORT_IDX_3_CNS;
    }

    rc = prvTgfPolicerPacketSizeModeSet (stage, packetSizeForRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerPacketSizeModeSet: %d, %d",
                                 stage, packetSizeForRestore);

    /* AUTODOC: Restore metering entry */
    rc = prvTgfPolicerEntrySet(stage,
                               prvTgfPortsArray[portNum],
                               &saveMeterEntry,
                               &tbParams);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                 stage,
                                 prvTgfPortsArray[portNum]);

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
                                            prvTgfPortsArray[portNum],
                                            savePortMeterEnable[stage]);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                           "prvTgfPolicerPortMeteringEnableSet: %d, %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 prvTgfPortsArray[portNum],
                                 savePortMeterEnable[stage]);

    /* AUTODOC: Restore metering mode */
    rc = prvTgfPolicerStageMeterModeSet(stage,
                                        saveMeterMode[stage]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerStageMeterModeSet: %d, %d",
                                 stage,
                                 saveMeterMode[stage]);

    /* AUTODOC: Restore counting mode */
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, stage,saveCountMode[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerCountingModeSet: %d, %d %d",
                                 prvTgfDevNum, stage, saveCountMode[stage]);

    /* AUTODOC: restore counting trigger by port */
    rc = prvTgfPolicerCountingTriggerByPortEnableSet(prvTgfDevNum,
                                                     stage,
                                                 saveCountTriggerByPort[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                      "prvTgfPolicerCountingTriggerByPortEnableSet: %d, %d, %d",
                                 prvTgfDevNum,
                                 stage,
                                 saveCountTriggerByPort[stage]);

    /* AUTODOC: restore MRU  */
    rc = prvTgfPolicerMruSet(stage, saveMru[stage]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerMruSet: %d, %d, %d",
                                 prvTgfDevNum, stage, saveMru[stage]);
}

/**
* @internal prvTgfPolicerBasicEarchMeterEntryTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPolicerBasicEarchMeterEntryTestGenerateTraffic
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS                       rc;
    GT_U32                          partsCount;
    GT_U32                          packetSize;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          plrMru;
    PRV_TGF_POLICER_ENTRY_STC           meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    PRV_TGF_POLICER_BILLING_ENTRY_STC   countEntry;
    GT_U32                              expMngCounters[4][3];
    GT_U32                              callIndex = 0;

    GT_U32          l1PacketSize;
    GT_U32          l3PacketSize;

    /* AUTODOC: IPv4-over-IPv4 traffic start */
    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketIpv4OverIpv4PartArray) /
                 sizeof(prvTgfPacketIpv4OverIpv4PartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketIpv4OverIpv4PartArray, partsCount,
                                                                   &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "IPv4oIPv4, prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketIpv4OverIpv4PartArray;

    l1PacketSize = packetSize + 4/*crc*/ + 8/*ipg*/ + 12/*preamble*/;
    l3PacketSize = packetSize - 12/*macs*/ - 4/*vlan*/ - 2/*ethertype*/;

    /* AUTODOC: set MRU value to be zero */
    plrMru = 0;

    mruSet(stage, plrMru);
    /* get updated MRU for calculations */
    plrMru = currentMru;

    /* Phase 0 */
    if(ALLOW_RUN_PHASE_MAC(0))
    {
        PRV_UTF_LOG0_MAC("Phase 0 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        /* AUTODOC: Set Metering based on packets per second */
        meterEntry.byteOrPacketCountingMode =
            PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;

        /* AUTODOC: Set TB to mark packet as RED */
        /* Packet is always out-of-profile if bucket size <= MRU */
        meterEntry.tbParams.srTcmParams.cbs = getCbs(0,meterEntry.byteOrPacketCountingMode);
        meterEntry.tbParams.srTcmParams.ebs = getEbs(0,meterEntry.byteOrPacketCountingMode);

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_RED);
        }

        /* AUTODOC: Send packet - RED */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet flooding */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /*          Red - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Red - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.redCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*0*/);
    }
    /* Phase 1 */
    if(ALLOW_RUN_PHASE_MAC(1))
    {
        PRV_UTF_LOG0_MAC("Phase 1 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        /* AUTODOC: Set Metering based on packets per second */
        meterEntry.byteOrPacketCountingMode =
            PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;
        /* AUTODOC: Set TB to mark packet as YELLOW */
        /* EBS should be >= 2 to be conforming because packet's byte count
           is 1 in packet mode */
        meterEntry.tbParams.srTcmParams.cbs = getCbs(0,meterEntry.byteOrPacketCountingMode);
        meterEntry.tbParams.srTcmParams.ebs = getEbs(2,meterEntry.byteOrPacketCountingMode);

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_YELLOW);
        }
        /* AUTODOC: Send packet - YELLOW */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet flooding */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /*          Yellow - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Yellow - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.yellowCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*1*/);
    }
    /* Phase 2 */
    if(ALLOW_RUN_PHASE_MAC(2))
    {
        PRV_UTF_LOG0_MAC("Phase 2 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        /* AUTODOC: Set Metering based on packets per second */
        meterEntry.byteOrPacketCountingMode =
            PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;
        /* AUTODOC: Set TB to mark packet as GREEN*/
        /* CBS should be >= 2 to be conforming because packet's byte count
           is 1 in packet mode */
        meterEntry.tbParams.srTcmParams.cbs = getCbs(2,meterEntry.byteOrPacketCountingMode);
        meterEntry.tbParams.srTcmParams.ebs = getEbs(2,meterEntry.byteOrPacketCountingMode);

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_GREEN);
        }
        /* AUTODOC: Send packet - GREEN */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet flooding */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /* AUTODOC: Green - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Green - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.greenCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*2*/);
    }
    /* Phase 3 */
    if(ALLOW_RUN_PHASE_MAC(3))
    {
        PRV_UTF_LOG0_MAC("Phase 3 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(debug_enable)/* previous phase was using 'packet resolutions' and
                           with next cbs,ebs */
        {
            /* Set Metering based on packets per second */
            meterEntry.byteOrPacketCountingMode =
                PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;
            /* Set TB to mark packet as GREEN*/
            /* CBS should be >= 2 to be conforming because packet's byte count
               is 1 in packet mode */
            meterEntry.tbParams.srTcmParams.cbs = getCbs(2,meterEntry.byteOrPacketCountingMode);
            meterEntry.tbParams.srTcmParams.ebs = getEbs(2,meterEntry.byteOrPacketCountingMode);
        }

        /* AUTODOC: Set Metering based on packets per second */
        meterEntry.byteOrPacketCountingMode =
            PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_RED);
        }
        /* AUTODOC: Send packet - RED */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet flooding */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /* AUTODOC: Red - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Red - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.redCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*3*/);
    }
    /* Phase 4 */
    if(ALLOW_RUN_PHASE_MAC(4))
    {
        PRV_UTF_LOG0_MAC("Phase 4 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        /*
            AUTODOC: Set TB to mark packet as GREEN\YELLOW\RED
                     for L3\L2\L1 respectively
        */
        meterEntry.tbParams.srTcmParams.cbs = getCbs((l3PacketSize + 1),meterEntry.byteOrPacketCountingMode);
        meterEntry.tbParams.srTcmParams.ebs = getEbs((l1PacketSize - 1),meterEntry.byteOrPacketCountingMode);

        /* AUTODOC: Set Metering packet size mode to L1 */
        meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_RED);
        }
        /* AUTODOC: Send packet - RED */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet flooding */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /* AUTODOC: Red - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_RED_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Red - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.redCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*4*/);
    }
    /* Phase 5 */
    if(ALLOW_RUN_PHASE_MAC(5))
    {
        PRV_UTF_LOG0_MAC("Phase 5 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(debug_enable)/* previous phase was using next info */
        {
            /* Set Metering based on packets per second */
            meterEntry.byteOrPacketCountingMode =
                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
            /*
                Set TB to mark packet as GREEN\YELLOW\RED
                         for L3\L2\L1 respectively
            */
            meterEntry.tbParams.srTcmParams.cbs = getCbs((l3PacketSize + 1),meterEntry.byteOrPacketCountingMode);
            meterEntry.tbParams.srTcmParams.ebs = getEbs((l1PacketSize - 1),meterEntry.byteOrPacketCountingMode);

            /* Set Metering packet size mode to L1 */
            meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;
        }

        /* AUTODOC: Set Red packet command to DROP */
        meterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_RED | PLUS_DROP);
        }
        /* AUTODOC: Send packet - RED */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet dropping */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      0, 0);

        /* AUTODOC: Expected management counters */
        /* AUTODOC: Drop - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Red - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.redCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*5*/);
    }
    /* Phase 6 */
    if(ALLOW_RUN_PHASE_MAC(6))
    {
        PRV_UTF_LOG0_MAC("Phase 6 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(debug_enable)/* previous phase was using next info */
        {
            /* Set Metering based on packets per second */
            meterEntry.byteOrPacketCountingMode =
                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
            /*
                Set TB to mark packet as GREEN\YELLOW\RED
                         for L3\L2\L1 respectively
            */
            meterEntry.tbParams.srTcmParams.cbs = getCbs((l3PacketSize + 1),meterEntry.byteOrPacketCountingMode);
            meterEntry.tbParams.srTcmParams.ebs = getEbs((l1PacketSize - 1),meterEntry.byteOrPacketCountingMode);

            /* Set Red packet command to DROP */
            meterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
        }


        /* AUTODOC: Set Metering packet size mode to L2 */
        meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_YELLOW);
        }
        /* AUTODOC: Send packet - YELLOW */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet flooding */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /* AUTODOC: Yellow - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_YELLOW_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /* AUTODOC: Yellow - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.yellowCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*6*/);
    }
    /* Phase 7 */
    if(ALLOW_RUN_PHASE_MAC(7))
    {
        PRV_UTF_LOG0_MAC("Phase 7 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(debug_enable)/* previous phase was using next info */
        {
            /* Set Metering based on packets per second */
            meterEntry.byteOrPacketCountingMode =
                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
            /*
                Set TB to mark packet as GREEN\YELLOW\RED
                         for L3\L2\L1 respectively
            */
            meterEntry.tbParams.srTcmParams.cbs = getCbs((l3PacketSize + 1),meterEntry.byteOrPacketCountingMode);
            meterEntry.tbParams.srTcmParams.ebs = getEbs((l1PacketSize - 1),meterEntry.byteOrPacketCountingMode);

            /* Set Red packet command to DROP */
            meterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;
        }

        /* AUTODOC: Set Metering packet size mode to L3 */
        meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_GREEN);
        }
        /* AUTODOC: Send packet - GREEN */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet flooding */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /* AUTODOC: Green - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_GREEN_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Green - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.greenCntr.l[0] = l3PacketSize;

        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*7*/);
    }
    /* Phase 8 */
    if(ALLOW_RUN_PHASE_MAC(8))
    {
        PRV_UTF_LOG0_MAC("Phase 8 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(debug_enable)/* previous phase was using next info */
        {
            /* Set Metering based on packets per second */
            meterEntry.byteOrPacketCountingMode =
                PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;
            /*
                Set TB to mark packet as GREEN\YELLOW\RED
                         for L3\L2\L1 respectively
            */
            meterEntry.tbParams.srTcmParams.cbs = getCbs((l3PacketSize + 1),meterEntry.byteOrPacketCountingMode);
            meterEntry.tbParams.srTcmParams.ebs = getEbs((l1PacketSize - 1),meterEntry.byteOrPacketCountingMode);

            /* Set Metering packet size mode to L3 */
            meterEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;
        }

        /* AUTODOC: Set Red packet command to 'no change' so we are sure that drop
           comes from green command */
        meterEntry.redPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
        /* AUTODOC: Set Green packet command to DROP */
        meterEntry.greenPcktCmd = PRV_TGF_POLICER_NON_CONFORM_CMD_DROP_E;

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_GREEN | PLUS_DROP);
        }
        /* AUTODOC: Send packet - GREEN */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_1_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet dropping */
        prvTgfPolicerEarchMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                      0, 0);

        /* AUTODOC: Expected management counters */
        /* AUTODOC: Drop - 1 packet, 122 bytes (L1) */
        cpssOsMemSet(&expMngCounters[0][0], 0, sizeof(expMngCounters));
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][0] = l1PacketSize;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][1] = 0;
        expMngCounters[PRV_TGF_POLICER_MNG_CNTR_DROP_E][2] = 1;

        /* AUTODOC: Expected billing counters */
        /*          Green - 1 packet, 80 bytes (L3) */
        cpssOsMemSet(&countEntry, 0, sizeof(PRV_TGF_POLICER_BILLING_ENTRY_STC));
        countEntry.greenCntr.l[0] = l3PacketSize;
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersCheck(stage,
                                           PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                           expMngCounters,
                                           prvTgfPortsArray[PRV_TGF_PORT_IDX_1_CNS],
                                           countEntry,
                                           callIndex++/*8*/);
    }
}

/**
* @internal prvTgfPolicerBasicEarchEgrMeterEntryTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results on egress port
*/
GT_VOID prvTgfPolicerBasicEarchEgrMeterEntryTestGenerateTraffic
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    GT_STATUS                           rc;
    TGF_PACKET_STC                      packetInfo;
    GT_U16                                mode;
    PRV_TGF_POLICER_ENTRY_STC           meterEntry;
    PRV_TGF_POLICER_METER_TB_PARAMS_UNT tbParams;
    GT_U32                              partsCount;
    GT_U32                              packetSize;
    GT_U32                              l2PacketSize;
    GT_U32                              l3PacketSize;
    GT_U32                              plrMru;

    /* AUTODOC: IPv4-over-IPv4 traffic start */
    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketIpv4OverIpv4PartArray) /
                 sizeof(prvTgfPacketIpv4OverIpv4PartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketIpv4OverIpv4PartArray, partsCount,
                             &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "IPv4oIPv4, prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketIpv4OverIpv4PartArray;

    l2PacketSize = packetSize + 4/*crc*/;
    l3PacketSize = packetSize - 12/*macs*/ - 4/*vlan*/ - 2/*ethertype*/;

    /* AUTODOC: set MRU value to be zero */
    plrMru = 0;
    mruSet(stage, plrMru);
    /* get updated MRU for calculations */
    plrMru = currentMru;

    /* Phase 0 */
    if(ALLOW_RUN_PHASE_MAC(0))
    {
        PRV_UTF_LOG0_MAC("Phase 0 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

        /* AUTODOC: Set Metering based on packets per second */
        meterEntry.byteOrPacketCountingMode =
            PRV_TGF_POLICER_METER_RESOLUTION_PACKETS_E;

        /* AUTODOC: Set TB to mark packet as YELLOW */
        meterEntry.tbParams.srTcmParams.cbs = getCbs(1,meterEntry.byteOrPacketCountingMode);
        meterEntry.tbParams.srTcmParams.ebs = getEbs(2,meterEntry.byteOrPacketCountingMode);

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                   stage, prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_YELLOW);
        }
        /* AUTODOC: Send packet - YELLOW (egress port) */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_3_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet receiving on egress port */
        prvTgfPolicerEarchEgrMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                         PRV_TGF_PORT_IDX_3_CNS,
                                                         prvTgfBurstCount, 0);

        /* AUTODOC: Expected management counters */
        /*          Yellow - 1 packet */
        prvTgfPolicerEarchEgrMeterEntryManagementCountCheck(
                                                 PRV_TGF_POLICER_MNG_CNTR_YELLOW_E);
    }

    /* Phase 1 */
    if(ALLOW_RUN_PHASE_MAC(1))
    {
        PRV_UTF_LOG0_MAC("Phase 1 \n");
        /* AUTODOC: Clear management & billing counters */
        prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                        PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                        prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

        rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                   stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS],
                                   &meterEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntryGet: %d, %d, %d",
                                     prvTgfDevNum,
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

        /* AUTODOC: Set Metering based on packets per second */
        meterEntry.byteOrPacketCountingMode =
            PRV_TGF_POLICER_METER_RESOLUTION_BYTES_E;

        /* Config meter entry with CBS = 1,  EBS = 2 for policer on egress port */
        if(PRV_TGF_POLICER_STAGE_EGRESS_E == stage)
        {
            meterEntry.tbParams.srTcmParams.cbs = getCbs(1,meterEntry.byteOrPacketCountingMode);
            meterEntry.tbParams.srTcmParams.ebs = getEbs(2,meterEntry.byteOrPacketCountingMode);
        }

        rc = prvTgfPolicerEntrySet(stage,
                                   prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS],
                                   &meterEntry,
                                   &tbParams);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                     stage,
                                     prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
        {
            /* call this before sending the packets ... because it sets the burst number */
            sip5_15_setBurstCount(EXPECT_RED);
        }
        /* AUTODOC: Send packet - RED */
        prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_3_CNS,
                                                   &packetInfo);

        /* AUTODOC: Verify packet receiving on egress port */
        prvTgfPolicerEarchEgrMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                         PRV_TGF_PORT_IDX_3_CNS,
                                                         prvTgfBurstCount, 0);
        /* AUTODOC: Expected management counters */
        /*          Red - 1 packet */
        prvTgfPolicerEarchEgrMeterEntryManagementCountCheck(
                                                    PRV_TGF_POLICER_MNG_CNTR_RED_E);
    }
    /* Phase 2 */
    if(ALLOW_RUN_PHASE_MAC(1))
    {
        PRV_UTF_LOG0_MAC("Phase 2 \n");
        for(mode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;
            mode <= CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E; mode++)
        {
            PRV_UTF_LOG1_MAC("Phase 2 , mode[%d] \n",mode);

            /* AUTODOC: Clear management & billing counters */
            prvTgfPolicerEarchMeterEntryManagementAndBillingCountersClear(stage,
                                          PRV_TGF_POLICER_MNG_CNTR_SET1_E,
                                          prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

            rc = prvTgfPolicerEntryGet(prvTgfDevNum,
                                       stage,
                                       prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS],
                                       &meterEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                         "prvTgfPolicerEntryGet: %d, %d, %d",
                                         prvTgfDevNum,
                                         stage,
                                         prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

            /*
                AUTODOC: Set TB to mark packet as GREEN\YELLOW\RED
                         for L3\L2\L1 respectively
            */
            meterEntry.tbParams.srTcmParams.cbs = getCbs((l3PacketSize + 1),meterEntry.byteOrPacketCountingMode);
            meterEntry.tbParams.srTcmParams.ebs = getEbs((l2PacketSize + 1),meterEntry.byteOrPacketCountingMode);

            /* AUTODOC: Set Metering packet size mode to L1 */
            meterEntry.packetSizeMode = mode;

            rc = prvTgfPolicerEntrySet(stage,
                                       prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS],
                                       &meterEntry,
                                       &tbParams);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerEntrySet: %d, %d",
                                         stage,
                                         prvTgfPortsArray[PRV_TGF_PORT_IDX_3_CNS]);

            rc = prvTgfPolicerPacketSizeModeSet (stage, mode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "prvTgfPolicerPacketSizeModeSet: %d, %d",
                                         stage, mode);

            if(PRV_CPSS_SIP_5_15_CHECK_MAC(prvTgfDevNum))
            {
                /* call this before sending the packets ... because it sets the burst number */
                sip5_15_setBurstCount(
                    mode == CPSS_POLICER_PACKET_SIZE_L3_ONLY_E      ? EXPECT_GREEN  :
                    mode == CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E   ? EXPECT_YELLOW :
                                                                      EXPECT_RED);
            }
            /* AUTODOC: Send packet */
            prvTgfPolicerEarchMeterEntryTestPacketSend(PRV_TGF_PORT_IDX_3_CNS,
                                                       &packetInfo);

            /* AUTODOC: Verify packet receiving on egress port */
            prvTgfPolicerEarchEgrMeterEntryTestCheckCounters(PRV_TGF_PORT_IDX_1_CNS,
                                                             PRV_TGF_PORT_IDX_3_CNS,
                                                             prvTgfBurstCount, 0);

            /* AUTODOC: Expected management counters */
            switch(mode)
            {
                case CPSS_POLICER_PACKET_SIZE_L3_ONLY_E:
                    /* AUTODOC: Expected management counters: Green - 1 packet */
                    prvTgfPolicerEarchEgrMeterEntryManagementCountCheck(
                                                  PRV_TGF_POLICER_MNG_CNTR_GREEN_E);
                    break;

                case CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E:
                    /* AUTODOC: Expected management counters: Yellow - 1 packet */
                    prvTgfPolicerEarchEgrMeterEntryManagementCountCheck(
                                                 PRV_TGF_POLICER_MNG_CNTR_YELLOW_E);
                    break;

                case CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E:
                    /* AUTODOC: Expected management counters: Red - 1 packet */
                    prvTgfPolicerEarchEgrMeterEntryManagementCountCheck(
                                                    PRV_TGF_POLICER_MNG_CNTR_RED_E);
                    break;

                 default:
                     break;
            }
        }
    }
}

/**
* @internal prvTgfPolicerBasicEarchMeterEntryTest function
* @endinternal
*
* @brief   Basic eArch metering entry test
*
* @param[in] stage                    - Policer stage
*                                       None
*/
GT_VOID prvTgfPolicerBasicEarchMeterEntryTest
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage
)
{
    if(stage > PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        /* will never happen ... but need to fix clockwork issues */
        return;
    }

    if(0 == ALLOW_RUN_STAGE_MAC((GT_U32)stage))
    {
        return;
    }

    /* AUTODOC: Test configurations */
    prvTgfPolicerBasicEarchMeterEntryConfigurationSet(stage);

    /* AUTODOC: Test traffic and checks */
    if(stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        prvTgfPolicerBasicEarchEgrMeterEntryTestGenerateTraffic(stage);
    }
    else
    {
        prvTgfPolicerBasicEarchMeterEntryTestGenerateTraffic(stage);
    }

    /* AUTODOC: Restore configurations */
    prvTgfPolicerBasicEarchMeterEntryRestore(stage);
}

/**
* @internal prvTgfPolicerEarchMeterEntryGlobalConfigurationSet function
* @endinternal
*
* @brief   Generic global configurations for eArch metering entry tests
*/
GT_VOID prvTgfPolicerEarchMeterEntryGlobalConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: Enable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_TRUE);

    /* AUTODOC: Create vlan on all ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfPolicerEarchMeterEntryGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID prvTgfPolicerEarchMeterEntryGlobalConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /****************/
    /* Tables Reset */
    /****************/

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                                         prvTgfDevNum, GT_TRUE);

    /* Invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: Disable "eArch" metering entry format parsing */
    rc = prvTgfPolicerEntryUseEarchEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPolicerEntryUseEarchEnable: %d",
                                 GT_FALSE);
}

/* function to debug specific section of the test */
void prvTgfPolicerBasicEarchEgrMeterEntry_debug_section(
    GT_U32  policerUnit, /* 0/1/2 --> iplr0/1/eplr */
    GT_U32  sectionId /* phase number 0..8 */
)
{
    debug_enable = GT_TRUE;
    debug_policerUnit = policerUnit;
    debug_sectionId = sectionId;
}

