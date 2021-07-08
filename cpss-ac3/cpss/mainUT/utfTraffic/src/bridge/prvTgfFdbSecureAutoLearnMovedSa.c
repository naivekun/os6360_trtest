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
* @file prvTgfFdbSecureAutoLearnMovedSa.c
*
* @brief Test packets with moved SA dropping with Secure Auto Learn.
*
* @version   2
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
#include <common/tgfIpGen.h>

#include <bridge/prvTgfFdbSecureAutoLearnMovedSa.h>


/******************************************************************************
 *                       Test configuration section                           *
 ******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS               1

/* default vlanId */
#define PRV_TGF_VLAN_ID_CNS                 1

/* default FDB portIdx */
#define PRV_TGF_FDB_PORT_IDX_CNS            0

/* portIdx to send traffic to */
#define PRV_TGF_TX_PORT_IDX_CNS             1

/* default number of packets to send */
static GT_U32  prvTgfBurstCount           = 1;

/* traffic Tx delay */
#define PRV_TGF_TX_DELAY                    100

/* default test stageNum */
#define PRV_TGF_STAGENUM_CNS                2

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {0, 1, 0, 0},  /* Rx count for generate trafic */
    {0, 1, 0, 0},  /* Tx count for generate trafic */
    {0, 1, 0, 0},  /* Rx count for generate trafic */
    {1, 1, 1, 1}   /* Tx count for generate trafic */
};

/* expected drop counters */
static GT_U32  prvTgfExpDropCounters;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x12, 0x34}                /* srcMac */
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
    CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT     mode;
    GT_BOOL                                 movedSaEnable;
    GT_BOOL                                 portAutoLearnEnable[3];
    GT_BOOL                                 inlifAutoLearnEnable[3];
} prvTgfRestoreCfg;

/******************************************************************************/

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbSecureAutoLearnMovedSaConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbSecureAutoLearnMovedSaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry        = {{0}};
    PRV_TGF_INLIF_ENTRY_STC     inlifEntry;

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

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
                 prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    /* store port auto learn mode */
    rc = prvTgfBrgFdbPortAutoLearnEnableGet(prvTgfDevNum, prvTgfPortsArray[0], &prvTgfRestoreCfg.portAutoLearnEnable[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableGet");
    rc = prvTgfBrgFdbPortAutoLearnEnableGet(prvTgfDevNum, prvTgfPortsArray[1], &prvTgfRestoreCfg.portAutoLearnEnable[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableGet");
    rc = prvTgfBrgFdbPortAutoLearnEnableGet(prvTgfDevNum, prvTgfPortsArray[2], &prvTgfRestoreCfg.portAutoLearnEnable[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableGet");

    /* AUTODOC: set port auto learn mode to enable for port [0] */
    rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, prvTgfPortsArray[0], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");

    /* AUTODOC: set port auto learn mode to enable for port [1] */
    rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, prvTgfPortsArray[1], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");

    /* AUTODOC: set port auto learn mode to enable for port [2] */
    rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, prvTgfPortsArray[2], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");

    /*store current inlif configuraton*/
    rc = prvTgfInlifEntryGet(PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[0], &inlifEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifEntryGet");
    prvTgfRestoreCfg.inlifAutoLearnEnable[0] = inlifEntry.autoLearnEnable;

    rc = prvTgfInlifEntryGet(PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[1], &inlifEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifEntryGet");
    prvTgfRestoreCfg.inlifAutoLearnEnable[1] = inlifEntry.autoLearnEnable;

    rc = prvTgfInlifEntryGet(PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[2], &inlifEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifEntryGet");
    prvTgfRestoreCfg.inlifAutoLearnEnable[2] = inlifEntry.autoLearnEnable;

    /* AUTODOC: set InLif Auto Learn to enable for port [0] */
    rc = prvTgfInlifAutoLearnEnableSet(prvTgfDevNum, PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[0], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifAutoLearnEnableSet");

    /* AUTODOC: set InLif Auto Learn to enable for port [1] */
    rc = prvTgfInlifAutoLearnEnableSet(prvTgfDevNum, PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[1], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifAutoLearnEnableSet");

    /* AUTODOC: set InLif Auto Learn to enable for port [2] */
    rc = prvTgfInlifAutoLearnEnableSet(prvTgfDevNum, PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[2], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifAutoLearnEnableSet");



    /* AUTODOC: add FDB entry with MAC 00:00:00:00:12:34, VLAN 1, port 0 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbMacEntrySet");

    /* save secure auto learn mode */
    rc = prvTgfBrgFdbSecureAutoLearnGet(&prvTgfRestoreCfg.mode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbSecureAutoLearnGet");

    /* AUTODOC: set Auto Secure Learn Mode to CPSS_MAC_SECURE_AUTO_LEARN_UNK_SOFT_DROP_E */
    rc = prvTgfBrgFdbSecureAutoLearnSet(CPSS_MAC_SECURE_AUTO_LEARN_UNK_SOFT_DROP_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbSecureAutoLearnSet");

    /* store Secure Auto Learn Moved SA status */
    rc = prvTgfBrgFdbSecureAutoLearnMovedSaEnableGet(&prvTgfRestoreCfg.movedSaEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbSecureAutoLearnMovedSaEnableGet");

    /* AUTODOC: set Secure Auto Learn Moved SA to enable */
    rc = prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet");

}

/**
* @internal prvTgfFdbSecureAutoLearnMovedSaTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbSecureAutoLearnMovedSaTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc              = GT_OK;
    GT_U32      portIter        = 0;
    GT_U32      expPacketLen    = 0;
    GT_U32      rxPacketsCount  = 0;
    GT_U32      txPacketsCount  = 0;
    GT_U32      stageNum        = 0;
    GT_U32      dropCount       = 0;

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

        /* AUTODOC: send 1 packet to port 8 */
        switch (stageNum) 
        {
            case 0:
                PRV_UTF_LOG0_MAC("\n========== Send packet to port 8"\
                                 " Moved SA enabled -> "\
                                 " <DROP_SOFT> =============\n\n");
                break;

            case 1:
                PRV_UTF_LOG0_MAC("\n========== Send packet to port 8 (FDB entry"\
                                 " Moved SA disabled -> "\
                                 " <FORWARD and FDB update> =============\n\n");

                rc = prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet(GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet");
                break;

            default:
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                             "Error: Bad parameters in prvTgf"\
                                             "FdbSecurityBreachMovedSaTrafficGenerate");
                return;
        }

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfEthCountersReset");


        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* AUTODOC: send 1 packet with SA = 12:34 on port 8 with: */
        /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:0:00:0, VLAN=1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, 
                                       prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]); 
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, 
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

        /* AUTODOC: verify traffic: packets are dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {

            switch (stageNum) 
            {
                case 0:
                    /* expected number of Rx packets */
                    rxPacketsCount = prvTgfPacketsCountRxTxArr[0][portIter];
                    /* expected number of Tx packets */
                    txPacketsCount = prvTgfPacketsCountRxTxArr[1][portIter];
                    break;

                case 1:
                    /* expected number of Rx packets */
                    rxPacketsCount = prvTgfPacketsCountRxTxArr[2][portIter];
                    /* expected number of Tx packets */
                    txPacketsCount = prvTgfPacketsCountRxTxArr[3][portIter];
                    break;

                default:
                    rc = GT_BAD_PARAM;
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                                 "Error: Bad parameters in prvTgf"\
                                                 "FdbSecurityBreachMovedSaTrafficGenerate");
                    return;
            }

            if(portIter == PRV_TGF_TX_PORT_IDX_CNS)
            {
                expPacketLen =  PRV_TGF_PACKET_LEN_CNS;
            }
            else
            {
                /* all ports untagged */
                expPacketLen =  PRV_TGF_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;
            }

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
            prvTgfExpDropCounters = 1;
        }
        else
        {
            prvTgfExpDropCounters = 0;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfExpDropCounters * prvTgfBurstCount,
                                 dropCount,
                                 "\n ERROR: Expected dropCounter = %d\n\n",
                                 prvTgfExpDropCounters * prvTgfBurstCount);

    }
}

/**
* @internal prvTgfFdbSecureAutoLearnMovedSaConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbSecureAutoLearnMovedSaConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* restore port auto learn mode */
    rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, prvTgfPortsArray[0], prvTgfRestoreCfg.portAutoLearnEnable[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");
    rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, prvTgfPortsArray[1], prvTgfRestoreCfg.portAutoLearnEnable[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");
    rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, prvTgfPortsArray[2], prvTgfRestoreCfg.portAutoLearnEnable[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");

    /* restore secure auto learn mode */
    rc = prvTgfBrgFdbSecureAutoLearnSet(prvTgfRestoreCfg.mode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbSecureAutoLearnSet");

    /* restore Secure Auto Learn Moved SA status */
    rc = prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet(prvTgfRestoreCfg.movedSaEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbSecureAutoLearnMovedSaEnableSet");

    /* restore InLif Auto Learn */
    rc = prvTgfInlifAutoLearnEnableSet(prvTgfDevNum, PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[0], prvTgfRestoreCfg.inlifAutoLearnEnable[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifAutoLearnEnableSet");

    rc = prvTgfInlifAutoLearnEnableSet(prvTgfDevNum, PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[1], prvTgfRestoreCfg.inlifAutoLearnEnable[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifAutoLearnEnableSet");

    rc = prvTgfInlifAutoLearnEnableSet(prvTgfDevNum, PRV_TGF_INLIF_TYPE_PORT_E, prvTgfPortsArray[2], prvTgfRestoreCfg.inlifAutoLearnEnable[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfInlifAutoLearnEnableSet");


    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");
}


