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
* @file prvTgfPortTxResourceHistogram.c
*
* @brief Resource Histigram Counters features testing
*
* @version   7
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/generic/cpssCommonDefs.h>


#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            2

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port number to receive traffic from */
#define PRV_TGF_RECIEVE_PORT_IDX_0_CNS  0

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_2_CNS     2

/* default number of packets to send */
static GT_U32   prvTgfBurstCount   = 100;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* srcMac */
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
    0x00, 0x00, 0x55, 0x55};

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

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS


/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* FDB MAC entry */
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;

/**
* @internal prvTgfPortTxResourceHistogramFdbConfigurationSet function
* @endinternal
*
* @brief   Set FDB configuration.
*/
static GT_VOID prvTgfPortTxResourceHistogramFdbConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc;                 /* return code */

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS];
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
    prvTgfMacEntry.key.key.macVlan.macAddr.arEther[5] = 0x11;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:11, VLAN 2, Port 0 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfBrgFdbMacEntrySet %d", GT_TRUE);
}

/**
* @internal prvTgfPortTxResourceHistogramFdbEntryDelete function
* @endinternal
*
* @brief   Delete FDB entry.
*/
static GT_VOID prvTgfPortTxResourceHistogramFdbEntryDelete
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: delete created FDB entry */
    rc = prvTgfBrgFdbMacEntryDelete(&(prvTgfMacEntry.key));
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in prvTgfBrgFdbMacEntrySet %d", GT_TRUE);
}

/**
* @internal prvTgfPortTxResourceHistogramVlanConfigurationSet function
* @endinternal
*
* @brief   Set Default Vlan Port MAC Counters configuration.
*/
GT_VOID prvTgfPortTxResourceHistogramVlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    prvTgfPortTxResourceHistogramFdbConfigurationSet();
}

static GT_U32  prvTgfTxQueueSize = 50;

GT_U32 prvTgfTxQueueSizeSet(GT_U32 newSize)
{
    GT_U32 oldSize = prvTgfTxQueueSize;
    prvTgfTxQueueSize = newSize;
    return oldSize;
}

/* maximal queue size for the test */
#define PRV_TGF_TX_RESOURCE_QUEUE_SIZE_CNS (prvTgfTxQueueSize)

/**
* @internal prvTgfPortTxResourceHistogramConfigurationSet function
* @endinternal
*
* @brief   Set configuration.
*/
GT_VOID prvTgfPortTxResourceHistogramConfigurationSet
(
    GT_VOID
)
{
    GT_U32      i, cntrValue;
    GT_STATUS   rc = GT_OK;

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        /* BC3 require minimum 70 descriptors to get test working */
        prvTgfTxQueueSize = 70;
    }

    /* AUTODOC: set HOL system mode */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_PORT_PORT_HOL_E);

    for (i = 0; i < 4; i++)
    {
        /* AUTODOC: set threshold for Histogram counter increment: */
        /* AUTODOC:   cntrNum0=0, cntrNum1=10, cntrNum2=20, cntrNum3=30 */
        rc = prvTgfPortTxResourceHistogramThresholdSet(prvTgfDevNum, i, i*10);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTxResourceHistogramThresholdSet: %d, %d, %d",
                                     prvTgfDevNum, i, i*10);

        /* AUTODOC: clear all histogram counters */
        rc = prvTgfPortTxResourceHistogramCounterGet(prvTgfDevNum, i, &cntrValue);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxResourceHistogramCounterGet: %d, %d",
                                     prvTgfDevNum, i);
    }
}

/**
* @internal prvTgfPortTxResourceHistogramRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPortTxResourceHistogramRestore
(
    GT_VOID
)
{
    GT_U32      i, cntrValue;
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: restore default FC system mode */
    rc = prvTgfPortFcHolSysModeSet(prvTgfDevNum, PRV_TGF_PORT_PORT_FC_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortFcHolSysModeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_PORT_PORT_FC_E);

    prvTgfPortTxResourceHistogramFdbEntryDelete();

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    for (i = 0; i < 4; i++)
    {
        /* AUTODOC: reset threshold for Histogram counter increment */
        rc = prvTgfPortTxResourceHistogramThresholdSet(prvTgfDevNum, i, 0);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPortTxResourceHistogramThresholdSet: %d, %d, %d",
                                     prvTgfDevNum, i, 0);

        /* AUTODOC: clear all histogram counters */
        rc = prvTgfPortTxResourceHistogramCounterGet(prvTgfDevNum, i, &cntrValue);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxResourceHistogramCounterGet: %d, %d",
                                     prvTgfDevNum, i);
    }
}

/**
* @internal prvTgfPortTxResourceHistogramPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPortTxResourceHistogramPacketSend
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
* @internal prvTgfPortTxResourceHistogramTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfPortTxResourceHistogramTrafficGenerate
(
    void
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portIter = 0, i, cntrValue;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          savePortMaxBuffLimit, savePortMaxDescrLimit;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS saveTailDropProfileTc0Params;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS saveTailDropProfileTc7Params;
    PRV_TGF_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS setTailDropProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   savePort0ProfileSet, savePort18ProfileSet;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* save & set profile settings */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaGet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                        &savePortMaxBuffLimit,
                                        &savePortMaxDescrLimit);
    UTF_VERIFY_EQUAL2_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaGet: %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E);

    /* AUTODOC: set tail drop profile PROFILE_1: */
    /* AUTODOC:   portMaxBuffLimit=1024, portMaxDescrLimit=1024 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                        1024, 1024);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                 1024, 1024);

    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                           0, &saveTailDropProfileTc0Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileGet: %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E, 0);

    rc = prvTgfPortTx4TcTailDropProfileGet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                           7, &saveTailDropProfileTc7Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileGet: %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E, 7);

    cpssOsMemSet(&setTailDropProfileParams, 0, sizeof(setTailDropProfileParams));

    setTailDropProfileParams.dp0MaxBuffNum  = PRV_TGF_TX_RESOURCE_QUEUE_SIZE_CNS;
    setTailDropProfileParams.dp0MaxDescrNum = PRV_TGF_TX_RESOURCE_QUEUE_SIZE_CNS;
    setTailDropProfileParams.dp1MaxBuffNum  = 40;
    setTailDropProfileParams.dp1MaxDescrNum = 40;
    setTailDropProfileParams.dp2MaxBuffNum  = 30;
    setTailDropProfileParams.dp2MaxDescrNum = 30;
    setTailDropProfileParams.tcMaxBuffNum   = 100;
    setTailDropProfileParams.tcMaxDescrNum  = 100;

    /* AUTODOC: set tail drop profiles limits for TC 0: */
    /* AUTODOC:   dp0MaxBuffNum=dp0MaxDescrNum=50 */
    /* AUTODOC:   dp1MaxBuffNum=dp1MaxDescrNum=40 */
    /* AUTODOC:   dp2MaxBuffNum=dp2MaxDescrNum=30 */
    /* AUTODOC:   tcMaxBuffNum=tcMaxDescrNum=100 */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                           0, &setTailDropProfileParams);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E, 0);

    /* AUTODOC: set tail drop profiles limits for TC 7: */
    /* AUTODOC:   dp0MaxBuffNum=dp0MaxDescrNum=50 */
    /* AUTODOC:   dp1MaxBuffNum=dp1MaxDescrNum=40 */
    /* AUTODOC:   dp2MaxBuffNum=dp2MaxDescrNum=30 */
    /* AUTODOC:   tcMaxBuffNum=tcMaxDescrNum=100 */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                           7, &setTailDropProfileParams);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E, 7);

    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS],
                                     &savePort0ProfileSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(rc, GT_OK, "prvTgfPortTxBindPortToDpGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS]);

    rc = prvTgfPortTxBindPortToDpGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                     &savePort18ProfileSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(rc, GT_OK, "prvTgfPortTxBindPortToDpGet: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS]);

    /* AUTODOC: bind port 0 to DROP_PROFILE_1 */
    rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS],
                                     CPSS_PORT_TX_DROP_PROFILE_1_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTxBindPortToDpSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS],
                                 CPSS_PORT_TX_DROP_PROFILE_1_E);

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* AUTODOC: disable transmission from TC queue 0 on port 0 */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS], 0, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxQueueTxEnableSet: %d, %d, %d, *d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS], 0, GT_FALSE);

    /* Sending unicast */
    PRV_UTF_LOG0_MAC("======= Sending unicast =======\n");

    /* Set DA MAC Address to be 00:00:00:00:00:11 */
    prvTgfPacketL2Part.daMac[0] = 0x00;
    prvTgfPacketL2Part.daMac[1] = 0x00;
    prvTgfPacketL2Part.daMac[2] = 0x00;
    prvTgfPacketL2Part.daMac[3] = 0x00;
    prvTgfPacketL2Part.daMac[4] = 0x00;
    prvTgfPacketL2Part.daMac[5] = 0x11;

    /* AUTODOC: send 100 UC packets from port 2 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortTxResourceHistogramPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], &prvTgfPacketInfo);
    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify traffic on all ports - no Tx traffic is expected */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_2_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    for (i = 0; i < 4; i++)
    {
        /* get counters */
        rc = prvTgfPortTxResourceHistogramCounterGet(prvTgfDevNum, i, &cntrValue);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxResourceHistogramCounterGet: %d, %d",
                                     prvTgfDevNum, i);
    }

    /* AUTODOC: enable transmission from TC queue 0 on port 0 */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS], 0, GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxQueueTxEnableSet: %d, %d, %d, *d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS], 0, GT_TRUE);

    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify Tx traffic on port 0: */
    /* AUTODOC:   DA=01:02:03:04:05:06, SA=00:00:00:00:22:22 */
    /* AUTODOC:   goodPktsSent=50 */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx counters */
        if (PRV_TGF_RECIEVE_PORT_IDX_0_CNS == portIter)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_TX_RESOURCE_QUEUE_SIZE_CNS, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* AUTODOC: bind port 2 to DROP_PROFILE_1 */
    rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                     CPSS_PORT_TX_DROP_PROFILE_1_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTxBindPortToDpSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                 CPSS_PORT_TX_DROP_PROFILE_1_E);

    /* AUTODOC: disable transmission from TC queue 7 on port 2 */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], 7, GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxQueueTxEnableSet: %d, %d, %d, *d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS], 0, GT_FALSE);

    /* AUTODOC: send 100 UC packets from port 2 with: */
    /* AUTODOC:   DA=00:00:00:00:11:11, SA=00:00:00:00:22:22, VID=2 */
    prvTgfPortTxResourceHistogramPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], &prvTgfPacketInfo);

    cpssOsTimerWkAfter(10);

    /* AUTODOC: verify histogram counters: */
    /* AUTODOC:   cntrNum0=49, cntrNum1=39, cntrNum2=29, cntrNum3=19 */
    for (i = 0; i < 4; i++)
    {
        /* get counters */
        rc = prvTgfPortTxResourceHistogramCounterGet(prvTgfDevNum, i, &cntrValue);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortTxResourceHistogramCounterGet: %d, %d",
                                     prvTgfDevNum, i);

        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_TX_RESOURCE_QUEUE_SIZE_CNS - 1 -i*10,
                                     cntrValue, "got other value then expected for counter %d",
                                     i);
    }

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: enable transmission from TC queue 7 on port 2 */
    rc = prvTgfPortTxQueueTxEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], 7, GT_TRUE);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxQueueTxEnableSet: %d, %d, %d, *d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS], 0, GT_TRUE);

    cpssOsTimerWkAfter(10);

    /* AUTODOC: restore default tail drop profile PROFILE_1 */
    rc = prvTgfPortTxTailDropProfileWithoutAlphaSet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                        savePortMaxBuffLimit,
                                        savePortMaxDescrLimit);
    UTF_VERIFY_EQUAL4_STRING_MAC(rc, GT_OK, "prvTgfPortTxTailDropProfileWithoutAlphaSet: %d, %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                 savePortMaxBuffLimit,
                                 savePortMaxDescrLimit);

    /* AUTODOC: restore  default tail drop profiles limits for TC 0 */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                           0, &saveTailDropProfileTc0Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E, 0);

    /* AUTODOC: restore  default tail drop profiles limits for TC 7 */
    rc = prvTgfPortTx4TcTailDropProfileSet(prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E,
                                           7, &saveTailDropProfileTc7Params);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTx4TcTailDropProfileSet: %d, %d, %d",
                                 prvTgfDevNum, CPSS_PORT_TX_DROP_PROFILE_1_E, 7);

    /* AUTODOC: restore bind port 0 to default DROP_PROFILE */
    rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS],
                                     savePort0ProfileSet);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTxBindPortToDpSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECIEVE_PORT_IDX_0_CNS],
                                 savePort0ProfileSet);

    /* AUTODOC: restore bind port 2 to default DROP_PROFILE */
    rc = prvTgfPortTxBindPortToDpSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                     savePort18ProfileSet);
    UTF_VERIFY_EQUAL3_STRING_MAC(rc, GT_OK, "prvTgfPortTxBindPortToDpSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
                                 savePort18ProfileSet);
}

