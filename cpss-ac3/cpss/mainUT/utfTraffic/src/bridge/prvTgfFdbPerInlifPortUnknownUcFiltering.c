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
* @file prvTgfFdbPerInlifPortUnknownUcFiltering.c
*
* @brief Verify UC packets filtering thru InLIF port commandset.
*
* @version   5
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
#include <common/tgfInlifGen.h>

#include <bridge/prvTgfFdbPerInlifPortUnknownUcFiltering.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS               1

/* default vlanId */
#define PRV_TGF_VLAN_ID_CNS                 2

/* default FDB portIdx */
#define PRV_TGF_FDB_PORT_IDX_CNS            1

/* portIdx to send traffic to */
#define PRV_TGF_TX_PORT_IDX_CNS             0

/* default inlif index in the table */
#define PRV_TGF_INLIF_IDX_CNS               0

/* default number of packets to send */
static GT_U32  prvTgfBurstCount           = 10;

/* default traffic Tx delay in mS */
#define PRV_TGF_TX_DELAY_CNS                100

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},  /* Rx count for unfiltered command trafic                */
    {1, 1, 1, 1},  /* Tx count for unfiltered command trafic                */
    {1, 0, 0, 0},  /* Rx count for DropHard command trafic with unknown DA  */
    {1, 0, 0, 0},  /* Tx count for DropHard command trafic with unknown DA  */
    {1, 0, 0, 0},  /* Rx count for DropHard command trafic with known DA    */
    {1, 1, 0, 0},  /* Tx count for DropHard command trafic with known DA    */
    {1, 0, 0, 0},  /* Rx count for Trap2CPU command trafic                  */
    {1, 0, 0, 0},  /* Tx count for Trap2CPU command trafic                  */
    {1, 0, 0, 0},  /* Rx count for Mirror2CPU command trafic                */
    {1, 1, 1, 1},  /* Tx count for Mirror2CPU command trafic                */
    {1, 0, 0, 0},  /* Rx count for Forward command trafic                   */
    {1, 1, 1, 1}   /* Tx count for Forward command trafic                   */
};

/* change DA command tests sequence */
static PRV_TGF_PACKET_CMD_ENT  prvTgfDaCommandTestSequence[] =
{
    CPSS_PACKET_CMD_NONE_E,
    CPSS_PACKET_CMD_DROP_HARD_E,
    CPSS_PACKET_CMD_DROP_HARD_E,
    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
    CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
    CPSS_PACKET_CMD_FORWARD_E
};

/* length of tests sequence */
#define PRV_TGF_SEQUENCE_LEN_CNS   \
    sizeof(prvTgfDaCommandTestSequence)/sizeof(prvTgfDaCommandTestSequence[0])

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[] =
{
    0x0FFFFF,   /* vfd[0]=1, vfd[1]=1 for unfiltered command trafic         */
    0x00,       /* vfd[0]=0, vfd[1]=0 for DropHard  with unknown DA         */
    0x0FFFFF,   /* vfd[0]=1, vfd[1]=1 for DropHard  with known DA           */
    0x00,       /* vfd[0]=0, vfd[1]=0 for Trap2CPU command trafic           */
    0x0FFFFF,   /* vfd[0]=1, vfd[1]=1 for Mirror2CPU command trafic         */
    0x0FFFFF    /* vfd[0]=1, vfd[1]=1 for Forward command trafic            */
};

/* default FDB entry macAddr */
static TGF_MAC_ADDR prvTgfFdbEntryMacAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

/* default unknown macAddr */
static TGF_MAC_ADDR prvTgfUnknownMacAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLAN_ID_CNS                           /* pri, cfi, VlanId */
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

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_INLIF_PORT_MODE_ENT     inlifMode;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbPerInlifPortUnknownUcFilteringConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnknownUcFilteringConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* AUTODOC: set PVID 2 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                 PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgVlanPortVidSet");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfFdbEntryMacAddr,
                                          PRV_TGF_VLAN_ID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
                                          GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");

    /* save inlif port mode */
    rc = prvTgfInlifPortModeGet(PRV_TGF_TX_PORT_IDX_CNS,
                                &prvTgfRestoreCfg.inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeGet");

    /* AUTODOC: set inlif mode to support single input logical interface for port 0  */
    rc = prvTgfInlifPortModeSet(PRV_TGF_TX_PORT_IDX_CNS,
                                   PRV_TGF_INLIF_PORT_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfInlifPortModeSet");
}

/**
* @internal prvTgfFdbPerInlifPortUnknownUcFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbPerInlifPortUnknownUcFilteringTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc              = GT_OK;
    GT_U32      portIter        = 0;
    GT_U32      expPacketLen    = 0;
    GT_U32      numTriggers     = 0;
    GT_U32      rxPacketsCount  = 0;
    GT_U32      txPacketsCount  = 0;
    GT_U32      stageNum        = 0;

    CPSS_INTERFACE_INFO_STC     portInterface   = {0};
    TGF_VFD_INFO_STC            vfdArray[2]     = {{0}};

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 6 stages: */
    for (stageNum = 0; stageNum < PRV_TGF_SEQUENCE_LEN_CNS; stageNum++ )
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficTableRxPcktTblClear");

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfEthCountersReset");

        /* print log message */
        switch (prvTgfDaCommandTestSequence[stageNum])
        {
            case CPSS_PACKET_CMD_NONE_E:
                PRV_UTF_LOG0_MAC("\n==== Traffic generate without filtering ====================\n");
                break;

            case CPSS_PACKET_CMD_DROP_HARD_E:
                PRV_UTF_LOG0_MAC("\n==== Traffic generate with filtering command <DROP_HARD> ====\n");
                break;

            case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                PRV_UTF_LOG0_MAC("\n==== Traffic generate with filtering command <TRAP2CPU> =====\n");
                break;

            case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
                PRV_UTF_LOG0_MAC("\n==== Traffic generate with filtering command <MIRROR2CPU> ===\n");
                break;

            case CPSS_PACKET_CMD_FORWARD_E:
                PRV_UTF_LOG0_MAC("\n==== Traffic generate with filtering command <FORWARD> ======\n");
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Illegal filtering command: %d",
                                         prvTgfDaCommandTestSequence[stageNum]);
        }

        /* set Mac addr */
        if (2 == stageNum)
        {
            PRV_UTF_LOG0_MAC("Send known UC packet\n");
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbEntryMacAddr,
                         sizeof(prvTgfPacketL2Part.daMac));
        }
        else
        {
            PRV_UTF_LOG0_MAC("Send unknown UC packet\n");
            cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfUnknownMacAddr,
                         sizeof(prvTgfPacketL2Part.daMac));
        }

        /* AUTODOC: set InLIF's filtering mode for unknown UC DA to: */
        /* AUTODOC:   1 stage - NONE */
        /* AUTODOC:   2 stage - DROP_HARD */
        /* AUTODOC:   3 stage - DROP_HARD */
        /* AUTODOC:   4 stage - TRAP_TO_CPU */
        /* AUTODOC:   5 stage - MIRROR_TO_CPU */
        /* AUTODOC:   6 stage - FORWARD */
        if (prvTgfDaCommandTestSequence[stageNum] != PRV_TGF_PACKET_CMD_NONE_E)
        {
            rc = prvTgfInlifUnkUnregAddrCmdSet(prvTgfDevNum,
                                               PRV_TGF_INLIF_TYPE_PORT_E,
                                               PRV_TGF_INLIF_IDX_CNS,
                                               PRV_TGF_INLIF_UNK_UC_DA_E,
                                               prvTgfDaCommandTestSequence[stageNum]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "Error of prvTgfInlifUnkUnregAddrCmdSet");
        }

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_PCL_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* set Tx delay */
        prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY_CNS);

        /* AUTODOC: for stages 1,4,5,6 send 10 unknown UC packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:01, SA=00:00:00:00:00:88, VID=2 */

        /* AUTODOC: for stages 2,3 send 10 unknown\known UC packets on port 0 with: */
        /* AUTODOC:   DAs=[00:00:00:00:00:01,00:00:00:00:00:02] */
        /* AUTODOC:   SA=00:00:00:00:00:88, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                       prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

        /* remove Tx delay */
        prvTgfSetTxSetup2Eth(0, 0);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_PCL_E,
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 stage - 10 packets received on ports 1,2,3 */
        /* AUTODOC:   2 stage - 10 packets received on port 1 */
        /* AUTODOC:   3 stage - 10 packets received on ports 1,2,3 */
        /* AUTODOC:   4 stage - traffic trapped to CPU */
        /* AUTODOC:   5 stage - 10 packets received on ports 1,2,3 */
        /* AUTODOC:   6 stage - 10 packets received on ports 1,2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (stageNum*2)][portIter];

            /* expected number of Rx packets */
            rxPacketsCount = prvTgfPacketsCountRxTxArr[0 + (stageNum*2)][portIter]+
                             ((PRV_TGF_CPORT_IDX_CNS == portIter) * txPacketsCount);

            /* expected packet size */
            expPacketLen =  PRV_TGF_PACKET_LEN_CNS -
                            (TGF_VLAN_TAG_SIZE_CNS * (PRV_TGF_TX_PORT_IDX_CNS != portIter));

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter],
                                        rxPacketsCount, txPacketsCount, expPacketLen,
                                        prvTgfBurstCount);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                         "ERROR of prvTgfEthCountersCheck"\
                                         "  Port=%02d  expectedRx=%02d"\
                                         "  expectedTx=%02d"\
                                         "  expectedLen=%02d\n",
                                         prvTgfPortsArray[portIter],
                                         rxPacketsCount, txPacketsCount,
                                         expPacketLen);
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
        PRV_UTF_LOG1_MAC("numTriggers = %d\n", numTriggers);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum,
                                     portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", prvTgfExpTriggersArr[stageNum]);

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpTriggersArr[stageNum],
                                     numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");
    }
}

/**
* @internal prvTgfFdbPerInlifPortUnknownUcFilteringConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbPerInlifPortUnknownUcFilteringConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore inlif port mode */
    rc = prvTgfInlifPortModeSet(PRV_TGF_TX_PORT_IDX_CNS,
                                prvTgfRestoreCfg.inlifMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfInlifPortModeSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate and reset vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate");
}


