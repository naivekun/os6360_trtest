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
* @file prvTgfCncGen.c
*
* @brief CPSS EXMXPM Centralized counters (CNC) Technology facility implementation.
*
* @version   15
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <cnc/prvTgfCncGen.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

#define PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(_dev)     \
    (GT_FALSE == prvUtfDeviceTestNotSupport(          \
        _dev, UTF_PUMA3_E))

#define PRV_TGF_CNC_GEN_XCAT_AND_ABOVE_MAC(_dev)      \
     (GT_FALSE == prvUtfDeviceTestNotSupport(         \
         _dev, (UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_CPSS_PP_E_ARCH_CNS)))

/* packet VID */
#define PRV_TGF_CNC_GEN_VLAN_CNS  5

/* block number */
#define PRV_TGF_CNC_GEN_BLOCK_NUM_CNS  1

/* counter number */
#define PRV_TGF_CNC_GEN_COUNTER_NUM_CNS  33

/* send port index */
#define PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS 1

/* receive port index */
#define PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS 2

/* default TC */
#define PRV_TGF_CNC_GEN_DEF_TC_CNS            5

/* default DP */
#define PRV_TGF_CNC_GEN_DEF_DP_CNS            CPSS_DP_YELLOW_E

/* used DxCh Qos profile */
#define PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0 0

/* used DP profile */
#define PRV_TGF_CNC_GEN_DEF_DP_PROFILE_CNS    CPSS_PORT_TX_DROP_PROFILE_7_E

/* sens port index */
#define PRV_TGF_CNC_GEN_BURST_COUNT_CNS 2

/* PCL rule index */
#define PRV_TGF_CNC_GEN_PCL_RULE_INDEX_CNS  8

/* PCL rule index */
#define PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS(lookup)  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet( \
    ((lookup) == CPSS_PCL_LOOKUP_1_E) ? 2 : 0 ,PRV_TGF_CNC_GEN_PCL_RULE_INDEX_CNS)

/* EPCL rule index */
#define PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_CNC_GEN_PCL_RULE_INDEX_CNS)


/* TTI rule index */
#define PRV_TGF_CNC_GEN_TTI_RULE_INDEX_CNS  12

/* saved VID */
static GT_U16 prvTgfDefVlanId = 1;

/* saved drop profile */
static PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS savedTailDropProfileParams = {0};

/* saved Bound Tail Drop Profile */
static CPSS_PORT_TX_DROP_PROFILE_SET_ENT   savedBoundTailDropProfile;

/* saved TTI MAC mode */
static PRV_TGF_TTI_MAC_MODE_ENT     savedTtiMacMode;

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

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfEthOthPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOthEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfEthOtherPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
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
* @internal prvTgfCncGenVidAndFdbDefCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfCncGenVidAndFdbDefCfgSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_CNC_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS], PRV_TGF_CNC_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: add FDB entry with MAC 00:11:22:33:44:00, VLAN 5, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_CNC_GEN_VLAN_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS], GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
};

/**
* @internal prvTgfCncGenVidAndFdbDefCfgRestore function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfCncGenVidAndFdbDefCfgRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

     /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* Vlan resrore */

    /* AUTODOC: restore default PVID on port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS], prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_CNC_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
};

/**
* @internal prvTgfCncGenEgrVlanTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] droppedPacket            - GT_TRUE   - dropped Packet
*                                      GT_FALSE  - passed Packet
*                                       None
*/
GT_VOID prvTgfCncGenEgrVlanTrafficGenerateAndCheck
(
    GT_BOOL droppedPacket
)
{
    GT_STATUS                         rc;
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    PRV_TGF_CNC_CHECK_STC             cncCheck;
    GT_U32                            numOfBytesInBuff;
    GT_U32                            blockSize;
    GT_U32                            numOfVlans;

    blockSize = prvTgfCncFineTuningBlockSizeGet();

    if (GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, (UTF_CPSS_PP_E_ARCH_CNS)))
    {
        /* eArch devices support 16K eVlan range for CNC */
        numOfVlans = _16K;
    }
    else
    {
        numOfVlans = _4K;
    }

    /* VLAN Configuration */

    rc = prvTgfCncGenVidAndFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenVidAndFdbDefCfgSet");

    if (droppedPacket != GT_FALSE)
    {
        /* Vlan contains only one port: ingress */
        GT_U32 portsArray[1];
        GT_U8 devArray[1];
        GT_U8 tagArray[1];

        portsArray[0] = prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS];
        devArray[0]   = prvTgfDevNum;
        tagArray[0]   = 0;

        /* invalidate and rewrite VLAN entry */
        rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_CNC_GEN_VLAN_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

        rc = prvTgfBrgDefVlanEntryWithPortsSet(
             PRV_TGF_CNC_GEN_VLAN_CNS,
             portsArray,
             devArray,
             tagArray,
             1 /*numPorts*/);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet");
    }

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_VLAN_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_TRUE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       =
        ((droppedPacket == GT_FALSE)
         ? 1
         : (1 << (numOfVlans / blockSize))); /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/
    /* prvTgfCncClientByteCountModeSet */
    cncCfg.configureByteCountMode = GT_TRUE;
    cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
    /* prvTgfCncEgressVlanDropCountModeSet */
    cncCfg.configureEgressVlanDropCountMode = (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE);
    cncCfg.egressVlanDropCountMode =
        PRV_TGF_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E;

    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

    /* forces Link Up on all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset");

    /* Traffic */
    rc = prvTgfTransmitPackets(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS,
        0 /*numVfd*/, NULL /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPackets");

    /* Check results */

    rc = prvTgfPacketSizeGet(
        prvTgfEthOtherPacketInfo.partsArray,
        prvTgfEthOtherPacketInfo.numOfParts,
        &numOfBytesInBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPacketSizeGet");

    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));
    /* common parameters */
    cncCheck.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCheck.counterNum    = PRV_TGF_CNC_GEN_VLAN_CNS;
    cncCheck.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCounterGet */
    cncCheck.compareCounterValue = GT_TRUE;
    cncCheck.counterValue.packetCount.l[0] =
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS;
    cncCheck.counterValue.byteCount.l[0] =
        (PRV_TGF_CNC_GEN_BURST_COUNT_CNS * (numOfBytesInBuff + 4 /*check sum*/));

    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* @internal prvTgfCncGenEgrVlanCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*/
GT_VOID prvTgfCncGenEgrVlanCfgRestore
(
    GT_VOID
)
{
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_VLAN_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    prvTgfCncGenConfigure(
        &cncCfg, GT_FALSE /*stopOnErrors*/);

    prvTgfCncGenVidAndFdbDefCfgRestore();
}

/**
* @internal prvTgfCncGenTxqTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] droppedPacket            - GT_TRUE   - dropped Packet
*                                      GT_FALSE  - passed Packet
* @param[in] cnMode                   - GT_TRUE   - CN messages count mode
*                                      GT_FALSE  - Tail Drop mode
*                                       None
*/
GT_VOID prvTgfCncGenTxqTrafficGenerateAndCheck
(
    GT_BOOL droppedPacket,
    GT_BOOL cnMode
)
{
    GT_STATUS                                    rc;
    PRV_TGF_CNC_CONFIGURATION_STC                cncCfg;
    PRV_TGF_CNC_CHECK_STC                        cncCheck;
    GT_U32                                       numOfBytesInBuff;
    GT_U32                                       txqIndex;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS   tailDropProfileParams;
    GT_U32                                       blockSize;
    CPSS_PORT_MAC_COUNTER_SET_STC                portCounters;

    blockSize = prvTgfCncFineTuningBlockSizeGet();

    rc = prvTgfCncGenVidAndFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenVidAndFdbDefCfgSet");

    txqIndex = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, cnMode, droppedPacket,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS] /*portNum*/,
        PRV_TGF_CNC_GEN_DEF_TC_CNS,
        PRV_TGF_CNC_GEN_DEF_DP_CNS);

    /* Qos Configuration */
    if (PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* Puma version */
        rc = prvTgfCosPortDefaultTcSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
            PRV_TGF_CNC_GEN_DEF_TC_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultTcSet");

        rc = prvTgfCosPortDefaultDpSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
            PRV_TGF_CNC_GEN_DEF_DP_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortDefaultDpSet");
    }
    else
    {
        /* DxCh version */
        CPSS_QOS_ENTRY_STC              qosEntry;
        PRV_TGF_COS_PROFILE_STC         qosProfile;

        cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));
        qosProfile.dropPrecedence = PRV_TGF_CNC_GEN_DEF_DP_CNS;
        qosProfile.trafficClass   = PRV_TGF_CNC_GEN_DEF_TC_CNS;

        cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
        qosEntry.assignPrecedence =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        qosEntry.enableModifyDscp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        qosEntry.enableModifyUp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        qosEntry.qosProfileId = PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0;

        rc = prvTgfCosProfileEntrySet(
            PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0, &qosProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

        rc = prvTgfCosPortQosConfigSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS], &qosEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

        rc = prvTgfCosPortQosTrustModeSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
            CPSS_QOS_PORT_NO_TRUST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosPortQosTrustModeSet");
    }

    /* cause tail drop */
    if (droppedPacket != GT_FALSE)
    {
        if (PRV_TGF_CNC_GEN_XCAT_AND_ABOVE_MAC(prvTgfDevNum))
        {
            /* disable flow control */
            prvTgfPortFcHolSysModeSet(
                prvTgfDevNum,
                PRV_TGF_PORT_PORT_HOL_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet");
            PRV_UTF_LOG0_MAC("flow control disabled\n");
        }

        /* save tail drop profile */
        rc = prvTgfPortTx4TcTailDropProfileGet(
            prvTgfDevNum,
            PRV_TGF_CNC_GEN_DEF_DP_PROFILE_CNS,
            PRV_TGF_CNC_GEN_DEF_TC_CNS,
            &savedTailDropProfileParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileGet");

        /* save bound tail drop profile index */
        rc = prvTgfPortTxBindPortToDpGet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
            &savedBoundTailDropProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpGet");

        /* cause tail drop */

        /* disable transmit from the egress port */
        rc = prvTgfPortTxQueueTxEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
            PRV_TGF_CNC_GEN_DEF_TC_CNS,
            GT_FALSE /*enable*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxQueueTxEnableSet");

        /* set zero-filled tail drop profile */
        cpssOsMemSet(&tailDropProfileParams, 0, sizeof(tailDropProfileParams));

        rc = prvTgfPortTx4TcTailDropProfileSet(
            prvTgfDevNum,
            PRV_TGF_CNC_GEN_DEF_DP_PROFILE_CNS,
            PRV_TGF_CNC_GEN_DEF_TC_CNS,
            &tailDropProfileParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTx4TcTailDropProfileSet");

        rc = prvTgfPortTxBindPortToDpSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
            PRV_TGF_CNC_GEN_DEF_DP_PROFILE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortTxBindPortToDpSet");
    }

    /* CNC Configuration */

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = (txqIndex % blockSize);
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_TRUE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       =
        (1 << (txqIndex / blockSize)); /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    if (! PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* for Puma3 it is TXQ_NETWORK client - counts always in L2 mode */
        /* prvTgfCncClientByteCountModeSet */
        cncCfg.configureByteCountMode = GT_TRUE;
        cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;

        /* prvTgfCncEgressQueueClientModeSet */
        if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            cncCfg.configureEgressQueueClientMode = GT_TRUE;
            cncCfg.egressQueueClientMode =
                (cnMode == GT_FALSE)
                   ? PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E
                   : PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
        }
    }

    PRV_UTF_LOG3_MAC(
        "blockSize %d, txqIndex %d, cncCfg.indexRangeBitMap[0] 0x%X\n",
        blockSize, txqIndex, cncCfg.indexRangeBitMap[0]);

    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

    /* forces Link Up on all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset");

    /* Traffic */
    rc = prvTgfTransmitPackets(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS,
        0 /*numVfd*/, NULL /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPackets");

    PRV_UTF_LOG0_MAC("Egress port counters:\n");
    /* dump egress port counters */
    rc = prvTgfReadPortCountersEth(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
        GT_TRUE /*enablePrint*/,
        &portCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfReadPortCountersEth");

    PRV_UTF_LOG0_MAC("Ingress port counters:\n");
    /* dump ingress port counters */
    rc = prvTgfReadPortCountersEth(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        GT_TRUE /*enablePrint*/,
        &portCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfReadPortCountersEth");

    /* Check results */

    rc = prvTgfPacketSizeGet(
        prvTgfEthOtherPacketInfo.partsArray,
        prvTgfEthOtherPacketInfo.numOfParts,
        &numOfBytesInBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPacketSizeGet");

    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));
    /* common parameters */
    cncCheck.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCheck.counterNum    = (txqIndex % blockSize);
    cncCheck.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCounterGet */
    cncCheck.compareCounterValue = GT_TRUE;
    cncCheck.counterValue.packetCount.l[0] =
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS;
    cncCheck.counterValue.byteCount.l[0] =
        (PRV_TGF_CNC_GEN_BURST_COUNT_CNS * numOfBytesInBuff);
    if (! PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* DX PPs count bytes with checksums, Puma3 - without checksums */
        cncCheck.counterValue.byteCount.l[0] +=
            (PRV_TGF_CNC_GEN_BURST_COUNT_CNS * 4); /*check sum*/
    }
    if (cnMode != GT_FALSE)
    {
        /* don't compare stamp */
        cncCheck.counterValue.byteCount.l[0] = 0xFFFFFFFF;
        cncCheck.counterValue.byteCount.l[1] = 0xFFFFFFFF;
    }

    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* @internal prvTgfCncGenTxqCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*
* @param[in] droppedPacket            - GT_TRUE   - dropped Packet
*                                      GT_FALSE  - passed Packet
* @param[in] cnMode                   - GT_TRUE   - CN messages count mode
*                                      GT_FALSE  - Tail Drop mode
*                                       None
*/
GT_VOID prvTgfCncGenTxqCfgRestore
(
    GT_BOOL droppedPacket,
    GT_BOOL cnMode
)
{
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    GT_STATUS                         rc, rc1 = GT_OK;
    GT_U32                            txqIndex;
    GT_U32                            blockSize;
    CPSS_PORT_MAC_COUNTER_SET_STC     portCounters;

    blockSize = prvTgfCncFineTuningBlockSizeGet();

    txqIndex = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, cnMode, droppedPacket,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS] /*portNum*/,
        PRV_TGF_CNC_GEN_DEF_TC_CNS,
        PRV_TGF_CNC_GEN_DEF_DP_CNS);

    /* Qos Configuration */
    if (PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* Puma version */
        rc = prvTgfCosPortDefaultTcSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
            0);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCosPortDefaultTcSet");

        rc = prvTgfCosPortDefaultDpSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
            CPSS_DP_GREEN_E);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfCosPortDefaultDpSet");
    }
    else
    {
        /* DxCh version */
        CPSS_QOS_ENTRY_STC              qosEntry;
        PRV_TGF_COS_PROFILE_STC         qosProfile;

        cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));

        cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
        qosEntry.assignPrecedence =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        qosEntry.enableModifyDscp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        qosEntry.enableModifyUp =
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        qosEntry.qosProfileId = 0;

        rc = prvTgfCosProfileEntrySet(
            PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0, &qosProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

        rc = prvTgfCosPortQosConfigSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS], &qosEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

        rc = prvTgfCosPortQosTrustModeSet(
            prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
            CPSS_QOS_PORT_NO_TRUST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosPortQosTrustModeSet");
    }


    /* cause drop on egress vlan filtering */
    if (droppedPacket != GT_FALSE)
    {
        if (PRV_TGF_CNC_GEN_XCAT_AND_ABOVE_MAC(prvTgfDevNum))
        {
            /* enable flow control */
            prvTgfPortFcHolSysModeSet(
                prvTgfDevNum,
                PRV_TGF_PORT_PORT_FC_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet");
            PRV_UTF_LOG0_MAC("flow control enabled\n");
        }

         /* enable transmit from the egress port */
        rc = prvTgfPortTxQueueTxEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
            PRV_TGF_CNC_GEN_DEF_TC_CNS,
            GT_TRUE /*enable*/);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPortTxQueueTxEnableSet");

        rc = prvTgfPortTx4TcTailDropProfileSet(
            prvTgfDevNum,
            PRV_TGF_CNC_GEN_DEF_DP_PROFILE_CNS,
            PRV_TGF_CNC_GEN_DEF_TC_CNS,
            &savedTailDropProfileParams);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPortTx4TcTailDropProfileSet");

        rc = prvTgfPortTxBindPortToDpSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
            savedBoundTailDropProfile);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPortTxBindPortToDpSet");

         /* dump egress port counters */
        PRV_UTF_LOG0_MAC("Egress port counters:\n");
        rc = prvTgfReadPortCountersEth(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
            GT_TRUE /*enablePrint*/,
            &portCounters);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfReadPortCountersEth");
   }

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = (txqIndex % blockSize);
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    if (! PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* prvTgfCncClientByteCountModeSet */
        cncCfg.configureByteCountMode = GT_TRUE;
        cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;

        /* prvTgfCncEgressQueueClientModeSet */
        if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&
            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&
            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {
            cncCfg.configureEgressQueueClientMode = GT_TRUE;
            cncCfg.egressQueueClientMode =
                PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
        }
    }

    prvTgfCncGenConfigure(
        &cncCfg, GT_FALSE /*stopOnErrors*/);

    prvTgfCncGenVidAndFdbDefCfgRestore();

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfCncGenIPclTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] useLookup1               - GT_FALSE = lookup0, GT_TRUE - lookup1
*                                       None
*/
GT_VOID prvTgfCncGenIPclTrafficGenerateAndCheck
(
    GT_BOOL useLookup1
)
{
    GT_STATUS                         rc;
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    PRV_TGF_CNC_CHECK_STC             cncCheck;
    GT_U32                            numOfBytesInBuff;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;
    CPSS_PCL_LOOKUP_NUMBER_ENT        lookup;

    rc = prvTgfCncGenVidAndFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenVidAndFdbDefCfgSet");

    lookup = (useLookup1 == GT_FALSE)
        ? CPSS_PCL_LOOKUP_0_E : CPSS_PCL_LOOKUP_1_E;

    /* PCL Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.egressPolicy = GT_FALSE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    if (PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* Puma version */
        action.policer.policerEnable =
            PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        action.policer.policerId = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    }
    else
    {
        /* DxCh version */
        action.matchCounter.enableMatchCount = GT_TRUE;
        action.matchCounter.matchCounterIndex =
            PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    }

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookup,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS(lookup), &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet");

    /* CNC Configuration */

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E;
    cncCfg.clientType    =
        (useLookup1 == GT_FALSE)
        ? PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E
        : PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;

    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_TRUE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       = 1; /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    if (! PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* prvTgfCncClientByteCountModeSet */
        cncCfg.configureByteCountMode = GT_TRUE;
        cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
    }

    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

    /* forces Link Up on all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset");

    /* Traffic */
    rc = prvTgfTransmitPackets(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS,
        0 /*numVfd*/, NULL /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPackets");

    /* Check results */

    rc = prvTgfPacketSizeGet(
        prvTgfEthOtherPacketInfo.partsArray,
        prvTgfEthOtherPacketInfo.numOfParts,
        &numOfBytesInBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPacketSizeGet");

    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));
    /* common parameters */
    cncCheck.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCheck.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCheck.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCounterGet */
    cncCheck.compareCounterValue = GT_TRUE;
    cncCheck.counterValue.packetCount.l[0] =
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS;
    cncCheck.counterValue.byteCount.l[0] =
        (PRV_TGF_CNC_GEN_BURST_COUNT_CNS * (numOfBytesInBuff + 4 /*check sum*/));

    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* @internal prvTgfCncGenIPclCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*
* @param[in] useLookup1               - GT_FALSE = lookup0, GT_TRUE - lookup1
*                                       None
*/
GT_VOID prvTgfCncGenIPclCfgRestore
(
    GT_BOOL useLookup1
)
{
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    GT_STATUS                         rc, rc1 = GT_OK;
    CPSS_PCL_LOOKUP_NUMBER_ENT        lookup;

    lookup = (useLookup1 == GT_FALSE)
        ? CPSS_PCL_LOOKUP_0_E : CPSS_PCL_LOOKUP_1_E;

     /* disable ingress policy on ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_CNC_GEN_IPCL_RULE_INDEX_CNS(lookup), GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E;
    cncCfg.clientType    =
        (useLookup1 == GT_FALSE)
        ? PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E
        : PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    prvTgfCncGenConfigure(
        &cncCfg, GT_FALSE /*stopOnErrors*/);

    rc = prvTgfCncGenVidAndFdbDefCfgRestore();

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfCncGenEPclTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncGenEPclTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                         rc;
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    PRV_TGF_CNC_CHECK_STC             cncCheck;
    GT_U32                            numOfBytesInBuff;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_ACTION_STC            action;

    /* AUTODOC: SETUP CONFIGURATION: */
    rc = prvTgfCncGenVidAndFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenVidAndFdbDefCfgSet");

    /* PCL Configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.egressPolicy = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    if (PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* Puma version */
        action.policer.policerEnable =
            PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        action.policer.policerId = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    }
    else
    {
        /* DxCh version */
        action.matchCounter.enableMatchCount = GT_TRUE;
        action.matchCounter.matchCounterIndex =
            PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    }

    /* AUTODOC: init PCL Engine for receive port 2: */
    /* AUTODOC:   egress direction, LOOKUP_0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule 8 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   action enableMatchCount, counterNum=33 */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet");

    /* CNC Configuration */

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_EGRESS_PCL_E;
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_PCL_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    if (PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        cncCfg.configureCountingUnitEnable = GT_TRUE;
        cncCfg.countingUnitEnable          = GT_TRUE;
    }

    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_TRUE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       = 1; /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    if (! PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* prvTgfCncClientByteCountModeSet */
        cncCfg.configureByteCountMode = GT_TRUE;
        cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
    }

    /* AUTODOC: set typical CNC configuration: */
    /* AUTODOC:   countingUnit=EGRESS_PCL,clientType=EGRESS_PCL,blockNum=1,counterNum=33 */
    /* AUTODOC:   counterFormat=MODE_0,indexRangeBitMap=1,byteCountMode=MODE_L2 */
    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

    /* forces Link Up on all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send 2 Ethernet packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   EtherType=0x3456 */
    rc = prvTgfTransmitPackets(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS,
        0 /*numVfd*/, NULL /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPackets");

    /* Check results */

    rc = prvTgfPacketSizeGet(
        prvTgfEthOtherPacketInfo.partsArray,
        prvTgfEthOtherPacketInfo.numOfParts,
        &numOfBytesInBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPacketSizeGet");

    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));
    /* common parameters */
    cncCheck.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCheck.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCheck.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCounterGet */
    cncCheck.compareCounterValue = GT_TRUE;
    cncCheck.counterValue.packetCount.l[0] =
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS;
    cncCheck.counterValue.byteCount.l[0] =
        (PRV_TGF_CNC_GEN_BURST_COUNT_CNS * (numOfBytesInBuff + 4 /*check sum*/));

    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) &&
        prvCpssDxChPortRemotePortCheck(prvTgfDevNum,
                                       prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS]))
    {
        /* packets egress via remote port with 4B FROM_CPU DSA tag */
        cncCheck.counterValue.byteCount.l[0] += PRV_TGF_CNC_GEN_BURST_COUNT_CNS *4;
    }
    /* AUTODOC: verify CNC counters for CNC format mode 0: */
    /* AUTODOC:   byteCount = 164 */
    /* AUTODOC:   packetCount = 2 */
    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* @internal prvTgfCncGenEPclCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*/
GT_VOID prvTgfCncGenEPclCfgRestore
(
    GT_VOID
)
{
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    GT_STATUS                         rc, rc1 = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disables egress policy on port 2 */
    rc = prvTgfPclEgressPclPacketTypesSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_RECEIVE_PORT_INDEX_CNS],
        PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: invalidate PCL rule 8 */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_CNC_GEN_EPCL_RULE_INDEX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_EGRESS_PCL_E;
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_PCL_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    if (PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        cncCfg.configureCountingUnitEnable = GT_TRUE;
        cncCfg.countingUnitEnable          = GT_FALSE;
    }
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(
        &cncCfg, GT_FALSE /*stopOnErrors*/);

    rc = prvTgfCncGenVidAndFdbDefCfgRestore();

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/**
* @internal prvTgfCncGenTtiTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncGenTtiTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                         rc;
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    PRV_TGF_CNC_CHECK_STC             cncCheck;
    GT_U32                            numOfBytesInBuff;
    PRV_TGF_TTI_RULE_UNT              pattern;
    PRV_TGF_TTI_RULE_UNT              mask;
    PRV_TGF_TTI_ACTION_STC            ruleAction;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: SETUP CONFIGURATION: */
    rc = prvTgfCncGenVidAndFdbDefCfgSet();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenVidAndFdbDefCfgSet");
     /* TTI Configuration */

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&ruleAction, 0, sizeof(ruleAction));

    ruleAction.command = CPSS_PACKET_CMD_FORWARD_E;
    if (PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* Puma version */
        ruleAction.meterEnable   = GT_FALSE;
        ruleAction.counterEnable = GT_TRUE;
        ruleAction.policerIndex  =  PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    }
    else
    {
        /* DxCh version*/
        ruleAction.bindToCentralCounter = GT_TRUE;
        ruleAction.centralCounterIndex  = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
        /* Tag1 modification - mandatory even when not relevant */
        ruleAction.tag1VlanCmd   = PRV_TGF_TTI_VLAN1_MODIFY_ALL_E;
        ruleAction.tag1UpCommand = PRV_TGF_TTI_TAG1_UP_ASSIGN_ALL_E;
    }

    /* AUTODOC: enable TTI lookup for port 1, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* save the current lookup Mac mode for Ethernet and DA */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, &savedTtiMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: add TTI rule 12 with: */
    /* AUTODOC:   key ETH, cmd FORWARD */
    /* AUTODOC:   action counterEnable, counterIdx=33 */
    /* AUTODOC:   action tag1VlanCmd=MODIFY_ALL, tag1UpCommand=ASSIGN_ALL */
    rc = prvTgfTtiRuleSet(
        PRV_TGF_CNC_GEN_TTI_RULE_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
        &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* CNC Configuration */

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E;
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_TTI_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;

    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_TRUE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       = 1; /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    if (! PRV_TGF_CNC_GEN_PUMA3_AND_ABOVE_MAC(prvTgfDevNum))
    {
        /* prvTgfCncClientByteCountModeSet */
        cncCfg.configureByteCountMode = GT_TRUE;
        cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;
    }

    /* AUTODOC: set typical CNC configuration: */
    /* AUTODOC:   countingUnit=TTI,clientType=TTI,blockNum=1,counterNum=33 */
    /* AUTODOC:   counterFormat=MODE_0,indexRangeBitMap=1,byteCountMode=MODE_L2 */
    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

    /* forces Link Up on all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: send 2 Ethernet packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   EtherType=0x3456 */
    rc = prvTgfTransmitPackets(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS,
        0 /*numVfd*/, NULL /*vfdArray*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPackets");

    /* Check results */

    rc = prvTgfPacketSizeGet(
        prvTgfEthOtherPacketInfo.partsArray,
        prvTgfEthOtherPacketInfo.numOfParts,
        &numOfBytesInBuff);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPacketSizeGet");

    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));
    /* common parameters */
    cncCheck.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCheck.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCheck.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCounterGet */
    cncCheck.compareCounterValue = GT_TRUE;
    cncCheck.counterValue.packetCount.l[0] =
        PRV_TGF_CNC_GEN_BURST_COUNT_CNS;
    cncCheck.counterValue.byteCount.l[0] =
        (PRV_TGF_CNC_GEN_BURST_COUNT_CNS * (numOfBytesInBuff + 4 /*check sum*/));

    /* AUTODOC: verify CNC counters for CNC format mode 0: */
    /* AUTODOC:   byteCount = 164 */
    /* AUTODOC:   packetCount = 2 */
    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* @internal prvTgfCncGenTtiCfgRestore function
* @endinternal
*
* @brief   Restore Configuration.
*/
GT_VOID prvTgfCncGenTtiCfgRestore
(
    GT_VOID
)
{
    PRV_TGF_CNC_CONFIGURATION_STC     cncCfg;
    GT_STATUS                         rc, rc1 = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* disables TTI per devices */
    /* AUTODOC: invalidate TTI rule 12 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_CNC_GEN_TTI_RULE_INDEX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: restore lookup MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, savedTtiMacMode);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiMacModeSet");

    /* AUTODOC: disable TTI lookup for TTI_KEY_ETH at the port 1 */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_CNC_GEN_SEND_PORT_INDEX_CNS],
        PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiPortLookupEnableSet");


    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E;
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_TTI_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = PRV_TGF_CNC_GEN_COUNTER_NUM_CNS;
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCountingEnableSet */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(
        &cncCfg, GT_FALSE /*stopOnErrors*/);

    rc = prvTgfCncGenVidAndFdbDefCfgRestore();

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

