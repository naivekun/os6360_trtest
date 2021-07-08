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
* @file prvTgfFdbSourceIdModeChange.c
*
* @brief Verify that when source ID assignment mode is default source ID,
* packets should filtered only if egress port is deleted from the
* default source ID that configured on ingress port.
* When egress port is deleted from the source ID group of the FDB entry,
* packets should not be filtered.
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

#include <common/tgfMirror.h>

#include <bridge/prvTgfFdbSourceIdModeChange.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS               3

/* default VLAN Id */
#define PRV_TGF_VLAN_ID_CNS                 2

/* test ports num */
#define PRV_TGF_TEST_PORTS_NUM_CNS          2

/* default ingress portIdx array */
static GT_U8 prvTgfIngressPortsIdxArr[PRV_TGF_TEST_PORTS_NUM_CNS] =
{
     0, 2
};

/* default sourceId for ingress ports array */
static GT_U8 prvTgfIngressSourceIdArr[PRV_TGF_TEST_PORTS_NUM_CNS] =
{
     2, 4
};

/* default egress portIdx array */
static GT_U8 prvTgfEgressPortsIdxArr[PRV_TGF_TEST_PORTS_NUM_CNS] =
{
     1, 3
};

/* default sourceId for egress ports array */
static GT_U8 prvTgfEgressSourceIdArr[PRV_TGF_TEST_PORTS_NUM_CNS] =
{
     1, 4
};

/* default number of used FDB mac entries */
#define PRV_TGF_FDB_MAC_ENTRIES_NUM_CNS     4

/* default number of packets to send */
static GT_U32  prvTgfBurstCount           = 1;

/* traffic Tx delay */
#define PRV_TGF_TX_DELAY                    300

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 1, 0},  /* Rx count for generate trafic */
    {1, 0, 1, 1},  /* Tx count for generate trafic */
    {1, 0, 1, 0},  /* Rx count for generate trafic */
    {1, 1, 1, 0}   /* Tx count for generate trafic */
};

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[] =
{
     0x03, 0x00  /* 011b, 00b */
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 2                                             /* pri, cfi, VlanId */
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
    GT_U32                          portsDefaultSourceId[PRV_TGF_TEST_PORTS_NUM_CNS];
} prvTgfRestoreCfg;

/******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfFdbSourceIdModeChangeIsIngressPort function
* @endinternal
*
* @brief   Check portIdx is ingress port
*
* @param[in] portIdx                  - port index number
*
* @retval GT_TRUE                  - on portIdx is an ingress port
* @retval GT_FALSE                 - on portIdx is not a ingress port
*/
static GT_BOOL prvTgfFdbSourceIdModeChangeIsIngressPort
(
    IN GT_U32      portIdx
)
{
    GT_U32          iIdx = 0;

    /* iterate thru all trunk members*/
    for (iIdx = 0; iIdx < PRV_TGF_TEST_PORTS_NUM_CNS; iIdx++ )
    {
        /* check port index is equal to McGroup member index */
        if (portIdx == prvTgfIngressPortsIdxArr[iIdx])
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbSourceIdModeChangeConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbSourceIdModeChangeConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      portIter        = 0;
    GT_U32                      ii              = 0;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry        = {{0}};
    TGF_MAC_ADDR                fdbEntryMacAddr = {0};

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    PRV_UTF_LOG0_MAC("\nSet Vlan configuration\n");

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgDefVlanEntryWrite");

    PRV_UTF_LOG0_MAC("\nAdd MAC entries\n");

    /* AUTODOC: add 4 FDB entries with: */
    /* AUTODOC:   MAC 00:00:00:00:00:01, VLAN 2, srcId 1, port 0 */
    /* AUTODOC:   MAC 00:00:00:00:00:02, VLAN 2, srcId 2, port 1 */
    /* AUTODOC:   MAC 00:00:00:00:00:03, VLAN 2, srcId 3, port 2 */
    /* AUTODOC:   MAC 00:00:00:00:00:04, VLAN 2, srcId 4, port 3 */
    for (portIter = 0; portIter < PRV_TGF_FDB_MAC_ENTRIES_NUM_CNS; portIter++ )
    {
        /* clear entry */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* fill mac entry by defaults */
        macEntry.key.entryType              = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLAN_ID_CNS;
        macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[portIter];

        macEntry.daCommand  = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand  = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.sourceId   = portIter+1;

        cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(fdbEntryMacAddr));
        /* set default MAC addr */
        fdbEntryMacAddr[5] = (GT_U8)(portIter + 1);
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        /* write MAC entry */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntrySet #%d",
                                     portIter+1);
    }

    PRV_UTF_LOG0_MAC("\nSource ID assignment mode: FDB_PORT_DEFAULT_E\n");

    /* AUTODOC: set srcID assignment mode FDB_PORT_DEFAULT for ports 0,2 */
    for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
    {
        /* set source ID assignment mode */
        rc = prvTgfBrgSrcIdPortAssignModeSet(prvTgfDevNum,
                                             prvTgfPortsArray[prvTgfIngressPortsIdxArr[ii]],
                                             CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortAssignModeSet");
    }

    PRV_UTF_LOG0_MAC("\nSet Source ID configuration\n");

    for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
    {
        /* store port's default source ID */
        rc = prvTgfBrgSrcIdPortDefaultSrcIdGet(prvTgfDevNum,
                                               prvTgfPortsArray[prvTgfIngressPortsIdxArr[ii]],
                                               &prvTgfRestoreCfg.portsDefaultSourceId[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdGet");

        /* AUTODOC: set default srcID 2,4 for ports 0,2 */
        rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                               prvTgfPortsArray[prvTgfIngressPortsIdxArr[ii]],
                                               prvTgfIngressSourceIdArr[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdSet");

        /* AUTODOC: delete ports 1,3 from srcId groups 1,4 */
        /* packets assigned with this source ID will not be forwarded to this port */
        rc = prvTgfBrgSrcIdGroupPortDelete(prvTgfDevNum,
                                           prvTgfEgressSourceIdArr[ii],
                                           prvTgfPortsArray[prvTgfEgressPortsIdxArr[ii]]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortDelete");
    }

    for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
    {
        /* AUTODOC: enable srcID egress filter for UC packets for ports 1,3 */
        rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum,
                                                    prvTgfPortsArray[prvTgfEgressPortsIdxArr[ii]],
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "Error of prvTgfBrgSrcIdPortUcastEgressFilterSet");
    }
}

/**
* @internal prvTgfFdbSourceIdModeChangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbSourceIdModeChangeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc             = GT_OK;
    GT_U32      portIter       = 0;
    GT_U32      expPacketLen   = 0;
    GT_U32      numTriggers    = 0;
    GT_U32      rxPacketsCount = 0;
    GT_U32      txPacketsCount = 0;
    GT_U32      ii             = 0;
    GT_U32      stageNum       = 0;

    CPSS_INTERFACE_INFO_STC     portInterface = {0};
    TGF_VFD_INFO_STC            vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < 2; stageNum++ )
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

        /* AUTODOC: for Stage#2 do the following: */
        if (stageNum == 1)
        {
            PRV_UTF_LOG0_MAC("\nSource ID assignment mode: PORT_DEFAULT_E\n");

            /* set CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E mode */
            for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
            {
                /* AUTODOC:   set srcID assignment mode PORT_DEFAULT for ports 0,2 */
                rc = prvTgfBrgSrcIdPortAssignModeSet(prvTgfDevNum,
                                                     prvTgfPortsArray[prvTgfIngressPortsIdxArr[ii]],
                                                     CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortAssignModeSet");
            }
        }

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset");

        /* clear entry */
        cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

        for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
        {
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                        TGF_CAPTURE_MODE_MIRRORING_E,
                                                        GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");


            prvTgfPacketL2Part.daMac[5] = (GT_U8)((ii*2) + 2);
            prvTgfPacketL2Part.saMac[5] = (GT_U8)((ii*2) + 1);

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                                     &prvTgfPacketInfo,
                                     prvTgfBurstCount, 0, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

            /* AUTODOC: send packet on port 0 with: */
            /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=2 */
            /* AUTODOC: send packet on port 2 with: */
            /* AUTODOC:   DA=00:00:00:00:00:04, SA=00:00:00:00:00:03, VID=2 */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                           prvTgfPortsArray[prvTgfIngressPortsIdxArr[ii]]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum,
                                     prvTgfPortsArray[prvTgfIngressPortsIdxArr[ii]]);

            /* disable capture */
            rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                        TGF_CAPTURE_MODE_MIRRORING_E,
                                                        GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                         prvTgfDevNum,
                                         portInterface.devPort.portNum);
        }

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   stage#1 - 1 packet on port 3 */
        /* AUTODOC:   stage#2 - 1 packet on port 1 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (stageNum*2)][portIter];

            /* expected number of Rx packets */
            rxPacketsCount = prvTgfPacketsCountRxTxArr[0 + (stageNum*2)][portIter]+
                             ((PRV_TGF_CPORT_IDX_CNS == portIter) * txPacketsCount);

            expPacketLen =  prvTgfFdbSourceIdModeChangeIsIngressPort(portIter) ?
                            PRV_TGF_PACKET_LEN_CNS :
                            PRV_TGF_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;

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
        cpssOsMemCpy(vfdArray[0].patternPtr,
                     prvTgfPacketL2Part.daMac,
                     sizeof(TGF_MAC_ADDR));
        /* get trigger counters where packet has MAC SA as macAddr */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr,
                     prvTgfPacketL2Part.saMac,
                     sizeof(TGF_MAC_ADDR));
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                            sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                            vfdArray,
                                                            &numTriggers);
        PRV_UTF_LOG1_MAC("numTriggers = %d\n", numTriggers);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum,
                                     portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", prvTgfExpTriggersArr[stageNum]);

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpTriggersArr[stageNum], numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");
    }
}

/**
* @internal prvTgfFdbSourceIdModeChangeConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbSourceIdModeChangeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;
    GT_U32      ii          = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable srcID egress filter for UC packets on ports 1,3 */
    for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
    {
        /* disable Source ID egress filter for unicast packets per Egress Port */
        rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum,
                                                    prvTgfPortsArray[prvTgfEgressPortsIdxArr[ii]],
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "Error of prvTgfBrgSrcIdPortUcastEgressFilterSet");
    }

    /* AUTODOC: restore srcId group 1,4 */
    for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
    {
        /* packets assigned with this source ID may be forwarded to this port */
        rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum,
                                           prvTgfEgressSourceIdArr[ii],
                                           prvTgfPortsArray[prvTgfEgressPortsIdxArr[ii]]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortAdd");
    }

    /* AUTODOC: restore default source ID on ports 0,2 */
    for (ii = 0; ii < PRV_TGF_TEST_PORTS_NUM_CNS; ii++)
    {
        rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                               prvTgfPortsArray[prvTgfIngressPortsIdxArr[ii]],
                                               prvTgfRestoreCfg.portsDefaultSourceId[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdSet");
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate: vlan %d",
                                 PRV_TGF_VLAN_ID_CNS);
}


