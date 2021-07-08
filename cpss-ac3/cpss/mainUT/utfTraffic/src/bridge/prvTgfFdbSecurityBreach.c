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
* @file prvTgfFdbSecurityBreach.c
*
* @brief Test packets with invalid SA dropping by Security Breach.
*
* @version   7
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

#include <bridge/prvTgfFdbSecurityBreach.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS               1

/* default vlanId */
#define PRV_TGF_VLAN_ID_CNS                 1

/* default FDB portIdx */
#define PRV_TGF_FDB_PORT_IDX_CNS            1

/* portIdx to send traffic to */
#define PRV_TGF_TX_PORT_IDX_CNS             0

/* default number of packets to send */
static GT_U32  prvTgfBurstCount           = 1;

/* traffic Tx delay */
#define PRV_TGF_TX_DELAY                    100

/* default test stageNum */
#define PRV_TGF_STAGENUM_CNS                2

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},  /* Rx count for generate trafic */
    {1, 0, 0, 0},  /* Tx count for generate trafic */
    {1, 0, 0, 0},  /* Rx count for generate trafic */
    {1, 0, 0, 0}   /* Tx count for generate trafic */
};

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[PRV_TGF_STAGENUM_CNS] =
{
     0x00, 0x00     /* both vfd[0] and vfd[1] zeroed, traffic dropped by Security Breach */
};

/* expected drop counters */
static GT_U32  prvTgfExpDropCounters = 1;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},               /* dstMac */
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11}                /* srcMac */
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
    CPSS_DROP_MODE_TYPE_ENT                     dropModeType;
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC    dropCntrCfg;
    PRV_TGF_BRG_DROP_CNTR_MODE_ENT              dropCntrMode;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbSecurityBreachConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbSecurityBreachConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry        = {{0}};
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC    cntrCfg = {0};

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    /* save drop mode */
    rc = prvTgfBrgSecurityBreachEventDropModeGet(prvTgfDevNum,
                                                 CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                                                 &prvTgfRestoreCfg.dropModeType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgSecurityBreachEventDropModeGet");

    /* AUTODOC: set DROP_MODE_SOFT for INVALID_MAC_SA event */
    rc = prvTgfBrgSecurityBreachEventDropModeSet(CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                                                 CPSS_DROP_MODE_SOFT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgSecurityBreachEventDropModeSet");

    /* save mode and it's configuration parameters for security breach */
    rc = prvTgfBrgSecurityBreachPortVlanDropCntrModeGet(prvTgfDevNum,
                                                        &prvTgfRestoreCfg.dropCntrCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgSecurityBreachPortVlanDropCntrModeGet");

    /* AUTODOC: set Security Breach Drop Counter for VLAN 1 */
    cntrCfg.dropCntMode = CPSS_BRG_SECUR_BREACH_DROP_COUNT_VLAN_E;
    cntrCfg.vlan = PRV_TGF_VLAN_ID_CNS;
    /* set drop counter mode */
    rc = prvTgfBrgSecurityBreachPortVlanDropCntrModeSet(&cntrCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgSecurityBreachPortVlanDropCntrModeSet");

    /* clear entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* select DA command - control (unconditionally trap to CPU) */
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_CNTL_E;

    /* fill mac entry by defaults for this task */
    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLAN_ID_CNS;
    macEntry.isStatic                       = GT_FALSE;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                 prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:11, VLAN 1, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

    /* save drop counter default mode */
    rc = prvTgfBrgCntDropCntrModeGet(&prvTgfRestoreCfg.dropCntrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgCntDropCntrModeGet");

    /* AUTODOC: set drop counter mode - drop with invalid MAC SA */
    rc = prvTgfBrgCntDropCntrModeSet(PRV_TGF_BRG_DROP_CNTR_INVALID_SA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgCntDropCntrModeSet");
}

/**
* @internal prvTgfFdbSecurityBreachTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbSecurityBreachTrafficGenerate
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
    GT_U32      dropCount       = 0;

    CPSS_INTERFACE_INFO_STC     portInterface   = {0};
    TGF_VFD_INFO_STC            vfdArray[2]     = {{0}};

    /* AUTODOC: GENERATE TRAFFIC: */

    for (stageNum = 0; stageNum < PRV_TGF_STAGENUM_CNS; stageNum++ )
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficTableRxPcktTblClear");

        /* clear the bridge ingress drop counter */
        rc = prvTgfBrgCntDropCntrSet(0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "Error of prvTgfBrgCntDropCntrSet");

        /* AUTODOC: for each packet with invalid MAC SA set drop mode SOFT and HARD */
        switch (stageNum) 
        {
            case 0:
                PRV_UTF_LOG0_MAC("\n========== Send packets with invalid SA"\
                                 " <DROP_SOFT> =============\n\n");
                break;

            case 1:
                PRV_UTF_LOG0_MAC("\n========== Send packets with invalid SA"\
                                 " <DROP_HARD> =============\n\n");
                rc = prvTgfBrgSecurityBreachEventDropModeSet(CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                                                             CPSS_DROP_MODE_HARD_E);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                             "Error of prvTgfBrgSecurityBreach"\
                                             "EventDropModeSet");
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                             "Error: Bad parameters in prvTgf"\
                                             "FdbSecurityBreachTrafficGenerate");
                return;
        }

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfEthCountersReset");

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

        /* AUTODOC: send 2 packets with invalid SA on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:11, SA=11:11:11:11:11:11, VLAN=1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                       prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]); 
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_PCL_E,
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* AUTODOC: verify traffic: packets are dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1][portIter];

            /* expected number of Rx packets */
            rxPacketsCount = prvTgfPacketsCountRxTxArr[0][portIter]+
                             ((PRV_TGF_CPORT_IDX_CNS == portIter) * txPacketsCount);

            expPacketLen =  PRV_TGF_PACKET_LEN_CNS;

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

        /* gets the bridge ingress drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "Error of prvTgfBrgCntDropCntrGet");

        /* AUTODOC: check drop counter for DROP_SOFT mode */
        PRV_UTF_LOG1_MAC("\nCurrent dropCounter = %d\n", dropCount);
        if (0 == stageNum)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfExpDropCounters * prvTgfBurstCount,
                                     dropCount,
                                     "\n ERROR: Expected dropCounter = %d\n\n",
                                     prvTgfExpDropCounters * prvTgfBurstCount);
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
* @internal prvTgfFdbSecurityBreachConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbSecurityBreachConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: clear the bridge ingress drop counter */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgCntDropCntrSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore port/vlan security breach drop counter */
    rc = prvTgfBrgSecurityBreachPortVlanDropCntrModeSet(&prvTgfRestoreCfg.dropCntrCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgSecurityBreachPortVlanDropCntrModeSet");

    /* AUTODOC: restore the drop mode for INVALID_MAC_SA security breach event */
    rc = prvTgfBrgSecurityBreachEventDropModeSet(CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
                                                 prvTgfRestoreCfg.dropModeType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgSecurityBreachEventDropModeSet");

    /* AUTODOC: restore drop counter mode */
    rc = prvTgfBrgCntDropCntrModeSet(prvTgfRestoreCfg.dropCntrMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgCntDropCntrModeSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");
}


