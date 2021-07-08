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
* @file prvTgfCncEgrQue.c
*
* @brief CPSS DXCH Centralized counters (CNC) Technology facility implementation.
*
* @version   11
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfCosGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <cnc/prvTgfCncEgrQue.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/******************************* Test packet **********************************/

/******************************* common payload *******************************/

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOthEtherTypePart =
{0x3456};

#if PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS
/* VLAN tag1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_CNC_TEST_UP0  /*pri*/,
    PRV_TGF_CNC_TEST_CFI0 /*cfi*/,
    PRV_TGF_CNC_TEST_VID0 /*vid*/
};
#endif /*PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS*/

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfEthOthPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
#if PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
#endif /*PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOthEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of ETH_OTHER packet */
#define PRV_TGF_ETH_OTHER_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + PRV_TGF_CNC_TEST_VLAN_TAG_SIZE_CNS \
    + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfEthOtherPacketInfo =
{
    PRV_TGF_ETH_OTHER_PACKET_LEN_CNS,              /* totalLen */
    (sizeof(prvTgfEthOthPacketPartArray)
        / sizeof(prvTgfEthOthPacketPartArray[0])), /* numOfParts */
    prvTgfEthOthPacketPartArray                    /* partsArray */
};

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCncTestEgrQueConfigure function
* @endinternal
*
* @brief   This function configures Cnc EgrQue tests
*/
GT_VOID prvTgfCncTestEgrQueConfigure
(
    GT_VOID
)
{
    GT_STATUS                      rc;
    GT_U32                         indexRangesBmp[4];
    PRV_TGF_CNC_COUNTER_STC        counter;

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfCncTestVidDefConfigurationSet();

    /* all ranges relevant to VID map into the block    */
    /* correct both for 512 and for 2048 counter blocks */
    indexRangesBmp[0] = 0xFF;
    indexRangesBmp[1] = 0;
    indexRangesBmp[2] = 0;
    indexRangesBmp[3] = 0;

    /* the TxQ index can be high ... need to add it's exact bit */
    /* this issue found at Falcon 6.4T device */
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        GT_U32  tailDropModeIdx = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, GT_FALSE /*cnMode*/, 0 /*droppedPacket*/,
        prvTgfPortsArray[PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS],
        0/*tc*/, 0/*dp*/);

        indexRangesBmp[0] |= 1 << (tailDropModeIdx / PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.cncBlockNumEntries);
    }



    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    /* AUTODOC: bind EGRESS_QUEUE_PASS_DROP client to counter block */
    /* AUTODOC: for EGRESS_QUEUE_PASS_DROP client set index range to 0xFF */
    /* AUTODOC: set FORMAT_MODE_0 format of CNC counter */
    rc = prvTgfCncTestCncBlockConfigure(
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncTestCncBlockConfigure");

    /* AUTODOC: enable clear by read mode of CNC counters read operation */
    rc = prvTgfCncCounterClearByReadEnableSet(
        GT_TRUE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterClearByReadEnableSet");

    /* AUTODOC: for FORMAT_MODE_0 set counter clear value 0 */
    rc = prvTgfCncCounterClearByReadValueSet(
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterClearByReadValueSet");

    if (GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* AUTODOC: for EGRESS_QUEUE_PASS_DROP CNC client set BYTE_COUNT_MODE_L2 mode */
        rc = prvTgfCncClientByteCountModeSet(
            PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,
            PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncClientByteCountModeSet");
    }

    return;
}

/**
* @internal prvTgfCncTestEgrQueTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestEgrQueTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          burstCount;
    GT_U32                          egrPort;
    GT_U32                          tailDropModeIdx;
    GT_U32                          cnModeIdx;
    CPSS_QOS_ENTRY_STC              qosEntry;
    PRV_TGF_COS_PROFILE_STC         qosProfile;
    GT_U32                          dp = 1;
    GT_U32                          tc = 0;
    GT_U32                          checkSumCount;
    GT_U32                          cncIncrPerPacket;
    CPSS_INTERFACE_INFO_STC         portInterface;

    if (GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* DX devices */
        checkSumCount = 4;
    }
    else
    {
        /* Puma3 devices - TXQ clients do not count check sum bytes */
        checkSumCount = 0;
    }

    if (prvUtfIsTrafficManagerUsed())
    {
        /* TxQ CNC counters use TxQ port. All ports connected to TM use same TxQ.
           Test use flooded traffic that exit from 3 tests ports. RECEIVE_PORT and
           another one use same TxQ and CNC counter will be incremented twice per packet.
           All ports connected to TM in Cetus. */
        cncIncrPerPacket = (IS_BOBK_DEV_CETUS_MAC(prvTgfDevNum)) ? 3 : 2;
    }
    else
    {
        cncIncrPerPacket = 1;
    }

    /* AUTODOC: GENERATE TRAFFIC: */
    egrPort = prvTgfPortsArray[PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS];

    tailDropModeIdx = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, GT_FALSE /*cnMode*/, 0 /*droppedPacket*/,
        egrPort, tc, dp);
    cnModeIdx = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, GT_TRUE /*cnMode*/, 0 /*droppedPacket*/,
        egrPort, tc, dp);

    /* restrict value */
    tailDropModeIdx %=
        PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.cncBlockNumEntries;
    cnModeIdx %=
        PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.cncBlockNumEntries;

    if (GT_FALSE != prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
    {
        /* DX devices */

        /* Set packets DP to 1, TC to 0 */
        qosProfile.dropPrecedence = dp;
        qosProfile.trafficClass   = tc;
        qosProfile.dscp           = 0;
        qosProfile.exp            = 0;
        qosProfile.userPriority   = 0;

        /* AUTODOC: config cos profile entry 0 with: */
        /* AUTODOC:   DP=YELLOW, UP=TC=DSCP=EXP=0 */
        rc = prvTgfCosProfileEntrySet(
            PRV_TGF_CNC_TEST_QOS_PROFILE0, &qosProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

        qosEntry.assignPrecedence =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        qosEntry.enableModifyDscp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        qosEntry.enableModifyUp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        qosEntry.qosProfileId = PRV_TGF_CNC_TEST_QOS_PROFILE0;

        /* AUTODOC: config QoS entry 0 for port 1 with: */
        /* AUTODOC:   assignPrecedenc=HARD */
        /* AUTODOC:   enableModifyUp=enableModifyDscp=DISABLE */
        rc = prvTgfCosPortQosConfigSet(
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], &qosEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");
    }
    else
    {
        /* Puma3 devices */

        rc = prvTgfCosPortDefaultTcSet(
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], tc);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultTcSet");

        rc = prvTgfCosPortDefaultDpSet(
            prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS], dp);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultDpSet");
    }

    /* AUTODOC: set Qos Trust Mode as NO_TRUST for port 1 */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS],
        CPSS_QOS_PORT_NO_TRUST_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosTrustModeSet");

    /* PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E */

    burstCount = 3;

    if(GT_FALSE != prvUtfDeviceTestNotSupport(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_PUMA_E))
    {
            /* AUTODOC: set TAIL_DROP egress queue client counting mode */
        rc = prvTgfCncEgressQueueClientModeSet(
            PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCncEgressQueueClientModeSet");
    }


    /* AUTODOC: send 3 Ethernet packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   EtherType=0x3456 */
    prvTgfCncTestSendPacketAndCheck(
        &prvTgfEthOtherPacketInfo,
        burstCount,
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        tailDropModeIdx,
        /* counting all packet bytes with CRC */
        ((prvTgfEthOtherPacketInfo.totalLen + checkSumCount) * burstCount),
        cncIncrPerPacket);
    /* AUTODOC: verify CNC counters: */
    /* AUTODOC:   byteCount = 246 */
    /* AUTODOC:   packetCount = 3 */

    if(GT_FALSE != prvUtfDeviceTestNotSupport(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_CH3_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E |
        UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E))
    {
        /* AUTODOC: Counting index for CPU port */
        tailDropModeIdx = prvTgfCncTxqClientIndexGet(
            prvTgfDevNum, GT_FALSE /*cnMode*/, 0 /*droppedPacket*/,
            28/*CPU Port*/, tc, 0);
        tailDropModeIdx %= PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->
            fineTuning.tableSize.cncBlockNumEntries;

        /* AUTODOC: Setup capture in order to produce egress packet on CPU port */
        /* AUTODOC: clear table */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        portInterface.type             = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum = prvTgfDevNum;
        portInterface.devPort.portNum  = prvTgfDevsArray[PRV_TGF_CNC_SEND_PORT_INDEX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet");

        prvTgfCncTestSendPacketAndCheck(
            &prvTgfEthOtherPacketInfo,
            burstCount,
            PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
            tailDropModeIdx,
            /* counting all packet bytes with CRC */
            ((prvTgfEthOtherPacketInfo.totalLen + checkSumCount) * burstCount),
            cncIncrPerPacket);

        /* AUTODOC: Deconfig capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet");
    }

    if (GT_FALSE == prvUtfDeviceTestNotSupport(
        prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E
        | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_PUMA_E))
    {
        return;
    }

    /* PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E */

    burstCount = 2;

    /* AUTODOC: set Congestion Notification egress queue client counting mode */
    rc = prvTgfCncEgressQueueClientModeSet(
        PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncEgressQueueClientModeSet");

    /* AUTODOC: send 2 Ethernet packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   EtherType=0x3456 */
    prvTgfCncTestSendPacketAndCheck(
        &prvTgfEthOtherPacketInfo,
        burstCount,
        PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC(),
        cnModeIdx,
        /* byte count is equal to burst count in this mode */
        burstCount,
        cncIncrPerPacket);
    /* AUTODOC: verify CNC counters: */
    /* AUTODOC:   byteCount = 2 */
    /* AUTODOC:   packetCount = 2 */
}




