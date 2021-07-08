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
* @file prvTgfBrgProtocolBasedVlanLlc.c
*
* @brief Protocol-Based VLAN LLC/Non-SNAP for ALL packets
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgProtocolBasedVlanLlc.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_5_CNS          5

/* VLAN Id Qos entry */
#define PRV_TGF_VLANID_6_CNS          6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port index to send traffic to */
#define PRV_TGF_SEND1_PORT_IDX_CNS    1

/* capturing port number */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS  2

/* number of send stages */
#define PRV_TGF_FDB_PORT_IDX_CNS      2

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS  3

/* entry number */
#define PRV_TGF_ENTRY_NUM_CNS         1

/* ether type */
#define PRV_TGF_ETHER_TYPE_CNS        1500

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x02, 0x22, 0x05, 0xdc, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfUntaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of UNTAGGED packet */
#define PRV_TGF_UNTAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* UNTAGGED PACKET to send */
static TGF_PACKET_STC prvTgfUntaggedPacketInfo =
{
    PRV_TGF_UNTAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfUntaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfUntaggedPacketPartArray                                        /* partsArray */
};

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/* type of traffic to send (tagged or untagged) */
static TGF_PACKET_STC* prvTgfPacketInfoArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    &prvTgfTaggedPacketInfo, &prvTgfUntaggedPacketInfo, &prvTgfTaggedPacketInfo
};

/* send ports array */
static GT_U8 prvTgfSendPort[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_SEND_PORT_IDX_CNS, PRV_TGF_SEND_PORT_IDX_CNS, PRV_TGF_SEND1_PORT_IDX_CNS
};

/* expected number of Rx packets on ports*/
static GT_U8 prvTgfPacketsCountRxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 1, 1, 0}
};

/* expected number of Tx packets on ports*/
static GT_U8 prvTgfPacketsCountTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 1, 1, 0}
};

/* expected number of Triggers on ports with index 2 */
static GT_U8 prvTgfPacketsCountTriggerArr[] =
{
    1, 1, 0
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgProtocolBasedVlanLlcConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgProtocolBasedVlanLlcConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[]  = {1, 1, 1, 1};

    PRV_TGF_BRG_VLAN_PROT_CLASS_CFG_STC    prvTgfVlanCfg;
    PRV_TGF_BRG_QOS_PROT_CLASS_CFG_STC     prvTgfQosCfg;
    CPSS_PROT_CLASS_ENCAP_STC              prvTgfEncList;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with tagged ports 0, 1, 2 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged port 0, 2 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* add port with index 2 to Vlan 6*/
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                 prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS]);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_VLANID_5_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: enable port protocol VLAN assignment on port 0 */
    rc = prvTgfBrgVlanPortProtocolVidEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortProtocolVidEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: config Vlan Protocol based classification to VID 6, packet attribute assign for all */
    cpssOsMemSet(&prvTgfVlanCfg, 0, sizeof(prvTgfVlanCfg));

    prvTgfVlanCfg.vlanId = PRV_TGF_VLANID_6_CNS;
    prvTgfVlanCfg.vlanIdAssignCmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    prvTgfVlanCfg.vlanIdAssignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    /* set QoS parameters */
    cpssOsMemSet(&prvTgfQosCfg, 0, sizeof(prvTgfQosCfg));

    /* AUTODOC: config QOS Protocol based classification to: */
    /* AUTODOC:   DP green, packet attribute assign for all, profile ID 1 */
    prvTgfQosCfg.qosAssignCmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    prvTgfQosCfg.qosParams.dp = CPSS_DP_GREEN_E;
    prvTgfQosCfg.qosParams.dscp = 0;
    prvTgfQosCfg.qosParams.exp = 0;
    prvTgfQosCfg.qosParams.tc = 0;
    prvTgfQosCfg.qosParams.up = 0;
    prvTgfQosCfg.qosParamsModify.modifyDp = GT_FALSE;
    prvTgfQosCfg.qosParamsModify.modifyDscp = GT_FALSE;
    prvTgfQosCfg.qosParamsModify.modifyExp = GT_FALSE;
    prvTgfQosCfg.qosParamsModify.modifyTc = GT_FALSE;
    prvTgfQosCfg.qosParamsModify.modifyUp = GT_FALSE;
    prvTgfQosCfg.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    prvTgfQosCfg.qosProfileId = PRV_TGF_ENTRY_NUM_CNS;

    /* write port protocol VID and QOS assignment values for port 0 */
    rc = prvTgfBrgVlanPortProtoVlanQosSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                          PRV_TGF_ENTRY_NUM_CNS, &prvTgfVlanCfg, &prvTgfQosCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortProtoVlanQosSet: %d, ", prvTgfDevNum);

    /* AUTODOC: enable non-SNAP LLC encapsulation formats */
    prvTgfEncList.ethV2 = GT_FALSE;
    prvTgfEncList.llcSnap = GT_FALSE;
    prvTgfEncList.nonLlcSnap = GT_TRUE;

    /* Write etherType and encapsulation of Protocol based classification */
    rc = prvTgfBrgVlanProtocolEntrySet(PRV_TGF_ENTRY_NUM_CNS,
                                       PRV_TGF_ETHER_TYPE_CNS, &prvTgfEncList);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanProtocolEntrySet");
}

/**
* @internal prvTgfBrgProtocolBasedVlanLlcTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgProtocolBasedVlanLlcTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      portIter        = 0;
    GT_U32                      packetLen       = 0;
    GT_U32                      sendIter        = 0;
    GT_U32                      numTriggers     = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    for (sendIter = 0; sendIter < PRV_TGF_SEND_PACKETS_NUM_CNS; sendIter++)
    {
        PRV_UTF_LOG0_MAC("======= Sending packets =======\n\n");

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: configure 3 packets: */
        /* AUTODOC:   1 tagged packet - DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   2 untagged packet - DA=00:00:00:00:34:02, SA=00:00:00:00:00:01 */
        /* AUTODOC:   3 tagged packet - DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter], prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

        /* AUTODOC: send traffic - 1,2 packets on port 0; 3 packet on port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* check ETH counter */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* calculate packet length */
            if(prvTgfPacketInfoArr[sendIter] ==  &prvTgfTaggedPacketInfo)
            {
                packetLen = prvTgfPacketInfoArr[sendIter]->totalLen;
            }
            else
            {
                packetLen = prvTgfPacketInfoArr[sendIter]->totalLen + TGF_VLAN_TAG_SIZE_CNS * (PRV_TGF_CAPTURE_PORT_IDX_CNS == portIter);
            };

            /* AUTODOC: verify to get traffic on port 2 */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        prvTgfPacketsCountRxArr[sendIter][portIter],
                                        prvTgfPacketsCountTxArr[sendIter][portIter],
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* get Trigger Counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS - 1;
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = PRV_TGF_VLANID_6_CNS;

        if(prvTgfPacketsCountTriggerArr[sendIter])
        {
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        }
        else
        {
            vfdArray[0].mode = TGF_VFD_MODE_STATIC_EXPECTED_NO_MATCH_E;
        }

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketsCountTriggerArr[sendIter], numTriggers,
                                     "get another trigger that expected: expected - %d, recieved - %d\n",
                                      prvTgfPacketsCountTriggerArr[sendIter], numTriggers);

    }
}

/**
* @internal prvTgfBrgProtocolBasedVlanLlcConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgProtocolBasedVlanLlcConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore etherType and encapsulation of Protocol based classification */
    rc = prvTgfBrgVlanProtocolEntryInvalidate(PRV_TGF_ENTRY_NUM_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanProtocolEntryInvalidate");

    /* AUTODOC: invalidate port protocol VLAN assignment to packets received on port 0 */
    rc = prvTgfBrgVlanPortProtocolVidEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: invalidate protocol based QoS and VLAN assignment on port 0 */
    rc = prvTgfBrgVlanPortProtoVlanQosInvalidate(prvTgfDevNum,
                                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                 PRV_TGF_ENTRY_NUM_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortProtoVlanQosInvalidate: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* invalidate VLAN 5 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* invalidate VLAN 6 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");
}


