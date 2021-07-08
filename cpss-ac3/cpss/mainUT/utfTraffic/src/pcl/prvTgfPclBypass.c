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
* @file prvTgfPclBypass.c
*
* @brief Specific PCL features testing
*
* @version   20
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclBypass.h>

/* max iteration's name length */
#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 100

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS          2

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS            5

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to forward traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS      1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  2

#define PRV_UTF_VERIFY_RC1(rc, name)                                         \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
        rc1 = rc;                                                            \
    }

/******************************* packet 1 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};
/* VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                             /* pri, cfi, VlanId */
};
/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xAA, 0xAA
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/******************************* packet 2 **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket2L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x04},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                 /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPacket2PayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPacket2PayloadDataArr),            /* dataLength */
    prvTgfPacket2PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacket2L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacket2PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacket2Info = {
    (TGF_L2_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket2PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket2PartArray                                        /* partsArray */
};


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

static const GT_CHAR prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "sending matched packet to port [%d] ...\n",
    "sending unmatched packet to port [%d] ...\n"
};
static TGF_PACKET_STC *prvTgfPacketInfoArray[] = {
    &prvTgfPacket1Info, &prvTgfPacket2Info
};

/* expected number of sent packets on PRV_TGF_FDB_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountRxFdb = 0;

/* expected number of transmitted packets on PRV_TGF_FDB_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountTxFdb = 1;

/* expected number of sent packets on PRV_TGF_RECEIVE_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountRx = 0;

/* expected number of transmitted packets on PRV_TGF_RECEIVE_PORT_NUM_CNS port */
static GT_U8 prvTgfPacketsCountTx = 1;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCountersEthReset function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfDefConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* create a vlan on ports (0,0), (0,8), (0,18), (0,23) */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* set default vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    return rc;
};

/**
* @internal prvTgfConfigurationRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* disable ingress policy on ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");

    /* invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    sendIter;

    /* reset counters */
    rc = prvTgfCountersEthReset();
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfCountersEthReset");

    /* sending two packets - matched and unmatched */
    for (sendIter = 0; sendIter < PRV_TGF_MAX_ITER_CNS; sendIter++)
    {
        PRV_UTF_LOG1_MAC(
            prvTgfIterationNameArray[sendIter],
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(
            prvTgfDevNum, prvTgfPacketInfoArray[sendIter],
            1 /*burstCount*/, 0 /*numVfd*/, NULL);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

        /* start transmitting */
        rc = prvTgfStartTransmitingEth(
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");
    }

    return rc;
};

/**
* @internal prvTgfTrafficPclRulesDefine function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficPclRulesDefine
(
    IN PRV_TGF_PCL_BYPASS_ENT         fieldIndex,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *maskPtr,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *pattPtr,
    INOUT PRV_TGF_PCL_ACTION_STC      *actionPtr
)
{
    GT_STATUS rc = GT_OK;

    /* mask for MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));
    cpssOsMemSet(maskPtr->ruleStdNotIp.macDa.arEther, 0xFF,
            sizeof(maskPtr->ruleStdNotIp.macDa.arEther));

    /* difine mask, pattern and action */
    cpssOsMemSet(pattPtr, 0, sizeof(*pattPtr));
    cpssOsMemCpy(pattPtr->ruleStdNotIp.macDa.arEther, prvTgfPacket1L2Part.daMac,
            sizeof(prvTgfPacket1L2Part.daMac));

    /* action redirect */
    cpssOsMemSet(actionPtr, 0, sizeof(*actionPtr));
    actionPtr->pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    actionPtr->redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    actionPtr->redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    PRV_UTF_LOG0_MAC("------- ");

    switch (fieldIndex)
    {
        case PRV_TGF_PCL_BYPASS_BRIDGE_E:
            PRV_UTF_LOG0_MAC("BYPASS_BRIDGE");
            actionPtr->bypassBridge = GT_TRUE;
            break;

        case PRV_TGF_PCL_BYPASS_INGRESS_PIPE_E:
            PRV_UTF_LOG0_MAC("BYPASS_INGRESS_PIPE");
            actionPtr->bypassIngressPipe = GT_TRUE;
            break;

        default:
            PRV_UTF_LOG1_MAC("!!! prvTgfTrafficPclRulesDefine: Unsupported fieldIndex = %d !!!\n", fieldIndex);
            return GT_BAD_PARAM;
    }

    PRV_UTF_LOG0_MAC(" TEST -------\n");


    return rc;
};

/**
* @internal prvTgfFdbMacLearningCheck function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfFdbMacLearningCheck
(
    IN TGF_MAC_ADDR macAddrArr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC     macEntryKey;

    /* get FDB entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, macAddrArr, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

    /* Checking if there is MAC learning */
    if (rc != GT_OK)
    {
        PRV_UTF_LOG6_MAC("Device has NOT learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                macAddrArr[0],macAddrArr[1],macAddrArr[2],macAddrArr[3],macAddrArr[4],macAddrArr[5]);
        return GT_FAIL;
    }

    /* compare entryType */
    rc = (macEntry.key.entryType == PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "(macEntry.key.entryType != MAC_ADDR_E");

    /* compare MAC address */
    rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
                      macEntryKey.key.macVlan.macAddr.arEther,
                      sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another MAC address");

    /* compare vlanId */
    rc = (macEntry.key.key.macVlan.vlanId == macEntryKey.key.macVlan.vlanId) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another vlanid");

    /* compare dstInterface.type */
    rc = (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another dstInterface.type");

    /* compare dstInterface */
    if (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) {
        rc = (macEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) ? GT_OK : GT_FAIL;
        PRV_UTF_VERIFY_GT_OK(rc, "FDB has another dstInterface.devPort.hwDevNum");
    }

    /* compare isStatic */
    rc = (macEntry.isStatic == GT_FALSE) ? GT_OK : GT_FAIL;
    if (rc == GT_OK)
        PRV_UTF_LOG6_MAC("Device has learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                macAddrArr[0],macAddrArr[1],macAddrArr[2],macAddrArr[3],macAddrArr[4],macAddrArr[5]);

    return rc;
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclBypass function
* @endinternal
*
* @brief   Set test configuration
*
* @param[in] fieldIndex               -
*                                       None
*/
GT_VOID prvTgfPclBypass
(
    IN PRV_TGF_PCL_BYPASS_ENT fieldIndex
)
{
    GT_STATUS                        rc = GT_OK;
    GT_STATUS                        rc1, rc2;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      patt;
    PRV_TGF_PCL_ACTION_STC           action;

    /* -------------------------------------------------------------------------
     * 1. Set common configuration
     */

    rc = prvTgfDefConfigurationSet();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfDefConfigurationSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* create a macEntry for packet 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket1L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* create a macEntry for packet 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacket2L2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* select two packets to send */
    prvTgfPacketInfoArray[0] = &prvTgfPacket1Info;
    prvTgfPacketInfoArray[1] = &prvTgfPacket2Info;

    /* -------------------------------------------------------------------------
     * 2. Set PCL rules
     */

    /* difine mask, pattern and action */
    rc = prvTgfTrafficPclRulesDefine(fieldIndex, &mask, &patt, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRulesDefine: %d, %d",
        prvTgfDevNum, fieldIndex);

    /* set PCL rule only if mask, patt and action will be correctly set */
    if (GT_OK == rc)
    {
        rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            prvTgfPclRuleIndex, &mask, &patt, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
            prvTgfDevNum, prvTgfPclRuleIndex, fieldIndex);
    }

    /* -------------------------------------------------------------------------
     * 3. Generate traffic and Check counters and MAC Learning
     */

    /* generate traffic */
    rc = prvTgfTrafficGenerate();

    /* check counter of FDB port */
    rc = prvTgfEthCountersCheck(prvTgfDevNum, 
        prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS],
        prvTgfPacketsCountRxFdb, prvTgfPacketsCountTxFdb,
        prvTgfPacketInfoArray[0]->totalLen, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    /* check counter of receive port */
    rc = prvTgfEthCountersCheck(prvTgfDevNum, 
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPacketsCountRx, prvTgfPacketsCountTx,
        prvTgfPacketInfoArray[0]->totalLen, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    /* check MAC Learning */
    rc1 = prvTgfFdbMacLearningCheck(prvTgfPacket1L2Part.saMac);
    rc2 = prvTgfFdbMacLearningCheck(prvTgfPacket2L2Part.saMac);

    PRV_UTF_LOG0_MAC("\nMAC Learning status is ");
    switch (fieldIndex)
    {
        case PRV_TGF_PCL_BYPASS_BRIDGE_E:
            rc = (rc1 == GT_OK) && (rc2 == GT_OK) ? GT_OK : GT_FAIL;

            if (rc == GT_OK)
                PRV_UTF_LOG0_MAC("CORRECT\n");
            else
            {
                PRV_UTF_LOG0_MAC("INCORRECT:\n\n");
                PRV_UTF_LOG0_MAC("BOTH MAC SA1 and MAC SA2 must be learned\n");
            }

            break;

        case PRV_TGF_PCL_BYPASS_INGRESS_PIPE_E:
            rc = (rc1 != GT_OK) && (rc2 == GT_OK) ? GT_OK : GT_FAIL;

            if (rc == GT_OK)
                PRV_UTF_LOG0_MAC("CORRECT\n");
            else
            {
                PRV_UTF_LOG0_MAC("INCORRECT:\n\n");
                PRV_UTF_LOG6_MAC("MAC SA %02X:%02X:%02X:%02X:%02X:%02X ONLY must be learned\n",
                    prvTgfPacket2L2Part.saMac[0],prvTgfPacket2L2Part.saMac[1],
                    prvTgfPacket2L2Part.saMac[2],prvTgfPacket2L2Part.saMac[3],
                    prvTgfPacket2L2Part.saMac[4],prvTgfPacket2L2Part.saMac[5]);
            }

            break;

        default:
            PRV_UTF_LOG1_MAC(
                "!!! prvTgfTrafficPclRulesDefine: Unsupported fieldIndex = %d !!!\n", fieldIndex);
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfFdbMacLearningCheck: rc1 = %d, rc2 = %d", rc1, rc2);

    /* -------------------------------------------------------------------------
     * 4. Restore configuration
     */
    rc = prvTgfConfigurationRestore();
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfConfigurationRestore: %d", prvTgfDevNum);
}



