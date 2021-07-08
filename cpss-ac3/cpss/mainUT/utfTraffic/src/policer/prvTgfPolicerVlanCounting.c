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
* @file prvTgfPolicerVlanCounting.c
*
* @brief Specific Policer features testing
*
* @version   10
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

#include <policer/prvTgfPolicerVlanCounting.h>

#include <common/tgfCommon.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfPolicerStageGen.h>
#include <common/tgfBridgeGen.h>

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            2

/* second VLAN Id */
#define PRV_TGF_SECOND_VLANID_CNS            3

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* port number to forward packet to */
#define PRV_TGF_FDB_PORT_IDX_CNS      0

/* port number to forward packet to */
#define PRV_TGF_FDB_PORT_1_IDX_CNS    2

/* port number to forward packet to */
#define PRV_TGF_FDB_PORT_2_IDX_CNS    3

/* default number of packets to send */
static GT_U32   prvTgfBurstCount   = 1;

#define   PRV_TGF_VLAN_TRIGGER_PKT_CMD_NUM_CNS  5

/* count of packets to send */
#define PRV_TGF_SEND_COUNT_CNS 5


/****************** packet 1 (IPv4 TCP matched packet) ********************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x22}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPolicerVlanFdbConfigurationSet function
* @endinternal
*
* @brief   Set FDB configuration.
*         MAC         VID   PORT    DA_CMD
*         00:00:00:00:00:10  2    0    CPSS_MAC_TABLE_FRWRD_E
*         00:00:00:00:00:11  2    18   CPSS_MAC_TABLE_DROP_E
*         00:00:00:00:00:12  2    23   CPSS_MAC_TABLE_CNTL_E
*         00:00:00:00:00:13  2    0    CPSS_MAC_TABLE_MIRROR_TO_CPU_E
*         00:00:00:00:00:14  2    18   CPSS_MAC_TABLE_SOFT_DROP_E
*/
static GT_STATUS prvTgfPolicerVlanFdbConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   prvTgfMacEntry;     /* FDB MAC entry */
    GT_STATUS                   rc;                 /* return code */

    /* Add 5 MAC Entries */

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    prvTgfMacEntry.isStatic                     = GT_TRUE;
    prvTgfMacEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.daRoute                      = GT_FALSE;
    prvTgfMacEntry.mirrorToRxAnalyzerPortEn     = GT_FALSE;
    prvTgfMacEntry.userDefined                  = 0;
    prvTgfMacEntry.daQosIndex                   = 0;
    prvTgfMacEntry.saQosIndex                   = 0;
    prvTgfMacEntry.daSecurityLevel              = 0;
    prvTgfMacEntry.saSecurityLevel              = 0;
    prvTgfMacEntry.appSpecificCpuCode           = GT_FALSE;
    prvTgfMacEntry.pwId                         = 0;
    prvTgfMacEntry.spUnknown                    = GT_FALSE;
    prvTgfMacEntry.sourceId                     = 1;

    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[1] = 0x0;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[2] = 0x0;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[3] = 0x0;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[4] = 0x0;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[5] = 0x10;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:10, VLAN 2, port 0, daCmd=FORWARD */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbMacEntrySet");


    /* 00:00:00:00:00:11 on port 18, with DA_cmd = CPSS_MAC_TABLE_DROP_E */
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_FDB_PORT_1_IDX_CNS];
    prvTgfMacEntry.daCommand = PRV_TGF_PACKET_CMD_DROP_HARD_E;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[5] = 0x11;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:11, VLAN 2, port 2, daCmd=DROP_HARD */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbMacEntrySet");

    /* 00:00:00:00:00:12 on port 23, DA_cmd = CPSS_MAC_TABLE_CNTL_E  */
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_FDB_PORT_2_IDX_CNS];
    prvTgfMacEntry.daCommand = PRV_TGF_PACKET_CMD_CNTL_E;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[5] = 0x12;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:12, VLAN 2, port 3, daCmd=CNTL */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbMacEntrySet");

    /* 00:00:00:00:00:13 on port 0, DA_cmd = CPSS_MAC_TABLE_MIRROR_TO_CPU_E  */
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    prvTgfMacEntry.daCommand = PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[5] = 0x13;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:13, VLAN 2, port 0, daCmd=MIRROR_TO_CPU */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbMacEntrySet");

    /* 00:00:00:00:00:14 on port 18, DA_cmd = CPSS_MAC_TABLE_SOFT_DROP_E  */
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_FDB_PORT_1_IDX_CNS];
    prvTgfMacEntry.daCommand = PRV_TGF_PACKET_CMD_DROP_SOFT_E;
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[5] = 0x14;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:14, VLAN 2, port 2, daCmd=DROP_SOFT */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgFdbMacEntrySet");

    return GT_OK;
}

/**
* @internal prvTgfDefPolicerVlanConfigurationSet function
* @endinternal
*
* @brief   Set Default Vlan Policer configuration.
*/
GT_VOID prvTgfDefPolicerVlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: create VLAN 2 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* Configure FDB entries */
    rc = prvTgfPolicerVlanFdbConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerVlanFdbConfigurationSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPolicerVlanConfigurationRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPolicerVlanConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);
    return;
}


/**
* @internal prvTgfDefPolicerVlanStageConfigurationSet function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfDefPolicerVlanStageConfigurationSet
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT  stage
)
{
    GT_STATUS rc = GT_OK;

    /* configure Vlan counting mode default values */
    rc = prvTgfPolicerVlanCountingStageDefValuesSet(prvTgfDevNum, stage,
                                                    PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerVlanCountingDefValuesSet");
}

/**
* @internal prvTgfPolicerVlanStageConfigurationRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPolicerVlanStageConfigurationRestore
(
    IN PRV_TGF_POLICER_STAGE_TYPE_ENT  stage
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore Policer VLAN 2 default values */
    rc = prvTgfPolicerVlanCountingStageDefValuesRestore(prvTgfDevNum, stage,
                                                        PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerVlanCountingDefValuesRestore");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfVlanPolicerTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfVlanPolicerTestPacketSend
(
    IN  GT_U32                        portNum,
    IN  TGF_PACKET_STC               *packetInfoPtr
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portsCount = prvTgfPortsNum;
    GT_U32          portIter   = 0;


    /* Send packet Vlan Tagged */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

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


    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);
}

/**
* @internal prvTgPolicerVlanStagefTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8, 5 packets:
*         macDa = 00:00:00:00:00:10,
*         macSa = 00:00:00:00:22:22,
*         Success Criteria:
*         5 Packet is captured on ports 0,18,23.
*         5 Packets are counted in Vlan counter [2].
*/
GT_VOID prvTgPolicerVlanStagefTrafficGenerate
(
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT    stage
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          cntrVal;    /* Policer Vlan counter value */
    GT_U32                          cntrValToExpect;    /* expected VLAN counter
                                                           value */
    TGF_VFD_INFO_STC                vfdArray[1]; /* Information about the burst
                                                    of packets */
    CPSS_PACKET_CMD_ENT             packetCmdArray[PRV_TGF_VLAN_TRIGGER_PKT_CMD_NUM_CNS];
    GT_U32                          i;      /* loop iterator */
    GT_U32                          burst_i;
    GT_U32                          expectedCount;/* expected count*/

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);


    /* -------------------------------------------------------------------------
     * 1. Sending unknown unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unknown unicast =======\n");

    /* AUTODOC: send Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfVlanPolicerTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify Ethernet packet on ports 0,2,3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }


        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* Check Policer VLAN counter [2] */
    rc = prvTgfPolicerVlanCntrGet(prvTgfDevNum, stage, PRV_TGF_VLANID_CNS, &cntrVal);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerVlanCntrGet FAILED, rc = [%d]", rc);
    }

    if(prvTgfNumOfPortGroups && usePortGroupsBmp == GT_FALSE && prvTgfDevPortForceNum)
    {
        /* the test added ring ports , and since the packet did flooding in
        the port groups , it add to vlan counting */
        expectedCount = prvTgfBurstCount * (prvTgfNumOfPortGroups - 1);
    }
    else
    {
        expectedCount = 0;
    }

    /* AUTODOC: verify Policer Vlan counters: */
    /* AUTODOC:   for egress stage = 4 */
    /* AUTODOC:   for other stages = 1 */
    if (stage == PRV_TGF_POLICER_STAGE_EGRESS_E)
    {
        expectedCount += (prvTgfBurstCount * 3);
        if (tgfTrafficGeneratorEtherTypeIgnoreGet() == GT_FALSE)
        {
            expectedCount += prvTgfBurstCount;
        }
        /* we have 3 egress packets on each ingress one
           + one packet from CPU */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedCount, cntrVal,
                                     "get another Policer Vlan counter than expected");
    }
    else
    {
        expectedCount += prvTgfBurstCount;
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedCount, cntrVal,
                                     "get another Policer Vlan counter than expected");
    }


    /* -------------------------------------------------------------------------
     * 2. Sending unknown unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unknown unicast =======\n");

    /* Send traffic with VLAN 3, verify that Policy VLAN counter [2] wasn't increased */
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_SECOND_VLANID_CNS;

    /* AUTODOC: create VLAN 3 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_SECOND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* Flush counters */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(stage);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerCountingWriteBackCacheFlush FAILED, rc = [%d]", rc);
    }

    /* Reset default Vlan counter */
    rc = prvTgfPolicerVlanCntrSet(prvTgfDevNum, stage, PRV_TGF_VLANID_CNS, 0);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerVlanCntrSet FAILED, rc = [%d]", rc);
    }

    /* AUTODOC: send Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=3 */
    prvTgfVlanPolicerTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(1);


    /* AUTODOC: verify Ethernet packet on ports 0,2,3 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }


        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* Check Policer VLAN counter [2] */
    rc = prvTgfPolicerVlanCntrGet(prvTgfDevNum, stage, PRV_TGF_VLANID_CNS,
                                    &cntrVal);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerVlanCntrGet FAILED, rc = [%d]", rc);
    }

    /* AUTODOC: verify Policer Vlan counters = 0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, cntrVal,
                                 "get another Policer Vlan counter than expected");

    /* Set back the VLAN ID */
    prvTgfPacketVlanTag0Part.vid = 2;

    /* Reset Second Vlan counter */
    rc = prvTgfPolicerVlanCntrSet(prvTgfDevNum, stage, PRV_TGF_SECOND_VLANID_CNS, 0);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerVlanCntrSet FAILED, rc = [%d]", rc);
    }

    /* AUTODOC: invalidate VLAN 3 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_SECOND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);


   /* -------------------------------------------------------------------------
     * 3. Sending 5 known unicast incremental packets, 5 times in a loop
     *    and each time another packet command is added to VLAN Trigger Cmd.
     */
    PRV_UTF_LOG0_MAC("======= Sending 5 known unicast =======\n");

    /* Set DA MAC Address to be 00:00:00:00:00:10 */
    prvTgfPacketL2Part.daMac[4] = 0x0;
    prvTgfPacketL2Part.daMac[5] = 0x10;

    /* send 5 packets with increment MAC DA */
    vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* set different packet commands in the array */
    packetCmdArray[0] = CPSS_PACKET_CMD_FORWARD_E;
    packetCmdArray[1] = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    packetCmdArray[2] = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    packetCmdArray[3] = CPSS_PACKET_CMD_DROP_HARD_E;
    packetCmdArray[4] = CPSS_PACKET_CMD_DROP_SOFT_E;

    for (i = 0; i < PRV_TGF_VLAN_TRIGGER_PKT_CMD_NUM_CNS; i++)
    {
        /* First time vlan counter will be 1 */
        cntrValToExpect = (i + 1);

        /* Flush counters */
        rc = prvTgfPolicerCountingWriteBackCacheFlush(stage);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerCountingWriteBackCacheFlush FAILED, rc = [%d]", rc);
        }

        /* Reset default Vlan counter */
        rc = prvTgfPolicerVlanCntrSet(prvTgfDevNum, stage, PRV_TGF_VLANID_CNS, 0);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerVlanCntrSet FAILED, rc = [%d]", rc);
        }

        /* The Trigger commands may be changed only for Ingress stages */
        if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E)
        {
            /* Update Vlan Trigger command bitmap */
            rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(prvTgfDevNum,
                                                                stage,
                                                                packetCmdArray[i],
                                                                GT_TRUE);
            /* Check packet cmd, when the cmd is DROP, both SOFT_DROP and
               HARD_DROP packets will be counted */
            if (packetCmdArray[i] == CPSS_PACKET_CMD_DROP_HARD_E)
            {
                /* two packets will be counted and not one */
                cntrValToExpect = (i + 2);
            }
        }
        else
        {
            /* What will be counted:
               - Forward FDB  +1
               - Soft, Hard Drop  +0
                 because packets will be dropped before egress
               - Trap:
                  BC2 B0 and above devices counts TO_CPU packets by default +1
                  Other devices +1
               - Mirror To CPU:
                  BC2 B0 and above devices counts TO_CPU packets by default +2
                  Other devices + 2 because this command produced two packets
               - all From CPU packets on Send port */
            cntrValToExpect = 4;
            if (tgfTrafficGeneratorEtherTypeIgnoreGet() == GT_FALSE)
            {
                cntrValToExpect += PRV_TGF_SEND_COUNT_CNS;
            }

            i = PRV_TGF_VLAN_TRIGGER_PKT_CMD_NUM_CNS;
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("[TGF]: prvTgfPolicerVlanCountingPacketCmdTriggerSet FAILED, rc = [%d], burst_i = [%d]", rc, i);
        }

        /* send 5 packets, with incremental MAC DA, starting from 00:00:00:00:00:10 */
        prvTgfPacketL2Part.daMac[5] = 0x10;

        /* AUTODOC: send one-by-one 5 Ethernet packets from port 1 with: */
        /* AUTODOC:   DA=00:00:00:00:00:[10..14], SA=00:00:00:00:22:22, VID=2 */
        for (burst_i = 0; burst_i < PRV_TGF_SEND_COUNT_CNS; burst_i++)
        {
            prvTgfVlanPolicerTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);
            cpssOsTimerWkAfter(1);

            /* increment MAC DA */
            prvTgfPacketL2Part.daMac[5] += 1;
        }

        /* Check Policer VLAN counter [2], verify that it was incremented by one */
        rc = prvTgfPolicerVlanCntrGet(prvTgfDevNum, stage,
                                        PRV_TGF_VLANID_CNS, &cntrVal);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfPolicerVlanCntrGet FAILED, rc = [%d]", rc);
        }

        /* AUTODOC: verify Policer Vlan counters after each send: */
        /* AUTODOC:   1,2,3,4,5 */
        UTF_VERIFY_EQUAL0_STRING_MAC(cntrValToExpect,
               cntrVal, "get another Policer Vlan counter than expected");
    }
}


