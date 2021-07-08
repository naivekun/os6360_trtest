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
* @file prvTgfFdbChangeDaCommand.c
*
* @brief Change DA Command. DA command can be defined as Drop,
* Forward, Mirror2CPU, Trap2CPU. The test verifies that
* all options of DA Command work properly.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <bridge/prvTgfFdbChangeDaCommand.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS         2

/* VLAN Id */
static TGF_VLAN_ID  prvTgfVlanIdArr[] =
{
    2, 3, 4, 5
};

/* port index to send traffic to */
static GT_U16  prvTgfSendPortIdxArr[] =
{
    1, 0, 0, 3
};

/* FDB entry port index */
static GT_U16  prvTgfFdbPortIdxArr[] =
{
    1, 2, 2, 1
};

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* expected captured packet number */
static GT_U32  prvTgfPacketNum  = 3;

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 1, 0, 0},  /* Rx count for Drop command trafic       */
    {0, 1, 0, 0},  /* Tx count for Drop command trafic       */
    {1, 0, 0, 0},  /* Rx count for Forward command trafic    */
    {1, 0, 1, 0},  /* Tx count for Forward command trafic    */
    {1, 0, 0, 0},  /* Rx count for Mirror2CPU command trafic */
    {1, 0, 1, 0},  /* Tx count for Mirror2CPU command trafic */
    {0, 0, 0, 1},  /* Rx count for Trap2CPU command trafic   */
    {0, 0, 0, 1}   /* Tx count for Trap2CPU command trafic   */
};

/* change DA command tests sequence */
static PRV_TGF_PACKET_CMD_ENT  prvTgfDaCommandTestSequence[] =
{
    PRV_TGF_PACKET_CMD_DROP_HARD_E,
    PRV_TGF_PACKET_CMD_FORWARD_E,
    PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E,
    PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E
};

/* length of tests sequence */
#define PRV_TGF_SEQUENCE_LEN_CNS   \
    sizeof(prvTgfDaCommandTestSequence)/sizeof(prvTgfDaCommandTestSequence[0])

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 0                                             /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                           /* dataLength */
    prvTgfPayloadDataArr                                    /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},      /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
   TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfFdbChangeDaCommandConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbChangeDaCommandConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;
    GT_U32      stageNum    = 0;

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    /* AUTODOC: create VLANs 2,3,4,5 with all untagged ports */
    for (stageNum = 0; stageNum < PRV_TGF_SEQUENCE_LEN_CNS; stageNum++)
    {
        rc = prvTgfBrgDefVlanEntryWrite(prvTgfVlanIdArr[stageNum]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite(%d)\n",
                                     prvTgfVlanIdArr[stageNum]);
    }
}

/**
* @internal prvTgfFdbChangeDaCommandTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbChangeDaCommandTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc             = GT_OK;
    GT_U32      portIter       = 0;
    GT_U32      stageNum       = 0;
    GT_U32      expPacketLen   = 0;
    GT_U32      numTriggers    = 0;
    GT_U32      expTriggers    = 0;
    GT_U32      rxPacketsCount = 0;
    GT_U32      txPacketsCount = 0;
    CPSS_MAC_TABLE_CMD_ENT  restoreSip6FdbSaDropCommand = CPSS_MAC_TABLE_SOFT_DROP_E;
    CPSS_MAC_TABLE_CMD_ENT  restoreSip6FdbDaDropCommand = CPSS_MAC_TABLE_SOFT_DROP_E;

    CPSS_INTERFACE_INFO_STC     portInterface = {0};
    TGF_VFD_INFO_STC            vfdArray[2];
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: GENERATE TRAFFIC: */

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require global configuration to interpret an FDB Drop comand */
        rc = cpssDxChBrgFdbSaDaCmdDropModeGet(
            prvTgfDevNum, &restoreSip6FdbSaDropCommand, &restoreSip6FdbDaDropCommand);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbSaDaCmdDropModeGet failed");
        rc = cpssDxChBrgFdbSaDaCmdDropModeSet(
            prvTgfDevNum, restoreSip6FdbSaDropCommand, CPSS_MAC_TABLE_DROP_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbSaDaCmdDropModeSet failed");
    }

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* iterate thru stages */
    for (stageNum = 0; stageNum < PRV_TGF_SEQUENCE_LEN_CNS; stageNum++)
    {
        /* print log message */
        switch (prvTgfDaCommandTestSequence[stageNum])
        {
            case PRV_TGF_PACKET_CMD_DROP_HARD_E:
                PRV_UTF_LOG0_MAC("\n========== Traffic generate to test DA command <DROP_HARD> =============\n\n");
                break;

            case PRV_TGF_PACKET_CMD_FORWARD_E:
                PRV_UTF_LOG0_MAC("\n========== Traffic generate to test DA command <FORWARD> ===============\n\n");
                break;

            case PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E:
                PRV_UTF_LOG0_MAC("\n========== Traffic generate to test DA command <MIRROR_TO_CPU> =========\n\n");
                break;

            case PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E:
                PRV_UTF_LOG0_MAC("\n========== Traffic generate to test DA command <TRAP_TO_CPU> ===========\n\n");
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Illegal DA command: %d",
                                             prvTgfDaCommandTestSequence[stageNum]);
        }

        PRV_UTF_LOG3_MAC("VlanID = %d , SendPortIdx = %d, FdbPortIdx = %d\n\n",
                         prvTgfVlanIdArr[stageNum],  prvTgfSendPortIdxArr[stageNum], prvTgfFdbPortIdxArr[stageNum]);

        /* clear entry */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* fill mac entry by defaults for this task */
        macEntry.key.entryType            = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId   = prvTgfVlanIdArr[stageNum];
        macEntry.isStatic                 = GT_FALSE;
        macEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.dstInterface.type        = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[prvTgfFdbPortIdxArr[stageNum]];

        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* select DA command from test sequence list */
        macEntry.daCommand                = prvTgfDaCommandTestSequence[stageNum];

        /* AUTODOC: add 4 FDB entries with MACs 00:00:00:00:00:01 and: */
        /* AUTODOC:   1. DA cmd=DROP_HARD, VLAN 2, port 1 */
        /* AUTODOC:   2. DA cmd=FORWARD, VLAN 3, port 2 */
        /* AUTODOC:   3. DA cmd=MIRROR_TO_CPU, VLAN 4, port 2 */
        /* AUTODOC:   4. DA cmd=TRAP_TO_CPU, VLAN 5, port 1 */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset: dev=%d", prvTgfDevNum);

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

        /* setup packet */
        prvTgfPacketVlanTagPart.vid = prvTgfVlanIdArr[stageNum];
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* send packet */
        /* AUTODOC: send 4 packets on ports [1, 0, 0, 3] with: */
        /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:11:11 */
        /* AUTODOC:   VID=[2,3,4,5] */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdxArr[stageNum]]);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfSendPortIdxArr[stageNum]]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum, portInterface.devPort.portNum);

        /* check ETH counter of ports */
        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 packet - drop */
        /* AUTODOC:   2 packet - forwarded on port 2 */
        /* AUTODOC:   3 packet - mirror to port 2 */
        /* AUTODOC:   4 packet - trapped to CPU */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (stageNum * 2)][portIter];

            /* expected number of Rx packets */
            rxPacketsCount =  prvTgfPacketsCountRxTxArr[0 + (stageNum * 2)][portIter];

            /* if txPacketsCount expected, add capturing to rxPacketCount */
            rxPacketsCount += ((txPacketsCount != 0) && (PRV_TGF_CPORT_IDX_CNS == portIter));

            /* expected packet size */
            expPacketLen =  PRV_TGF_PACKET_LEN_CNS -
                            (TGF_VLAN_TAG_SIZE_CNS * (prvTgfSendPortIdxArr[stageNum] != portIter));

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter],
                                        rxPacketsCount, txPacketsCount, expPacketLen,
                                        prvTgfBurstCount);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck  Port=%02d  ExpectedRx=%02d  ExpectedTx=%02d BurstCnt=%d\n",
                                        prvTgfPortsArray[portIter],
                                        rxPacketsCount, txPacketsCount,
                                        prvTgfBurstCount);
        }

        /* get trigger counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as macAddr */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters where packet has MAC SA as macAddr */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                            sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                            vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* expected number of Rx packets */
        expTriggers = prvTgfPacketNum * (prvTgfPacketsCountRxTxArr[1 + (stageNum * 2)][PRV_TGF_CPORT_IDX_CNS] != 0);

        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", expTriggers);

        UTF_VERIFY_EQUAL0_STRING_MAC(expTriggers, numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");

        /* AUTODOC: flush FDB entry for each iteration: */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* SIP_6 devices require global configuration to interpret an FDB Drop comand */
        rc = cpssDxChBrgFdbSaDaCmdDropModeSet(
            prvTgfDevNum, restoreSip6FdbSaDropCommand, restoreSip6FdbDaDropCommand);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbSaDaCmdDropModeSet failed");
    }
}

/**
* @internal prvTgfFdbChangeDaCommandConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbChangeDaCommandConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;
    GT_U32      stageNum    = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    for (stageNum = 0; stageNum < PRV_TGF_SEQUENCE_LEN_CNS; stageNum++)
    {
        rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanIdArr[stageNum]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgDefVlanEntryInvalidate: vlan %d",
                                     prvTgfVlanIdArr[stageNum]);
    }
}


