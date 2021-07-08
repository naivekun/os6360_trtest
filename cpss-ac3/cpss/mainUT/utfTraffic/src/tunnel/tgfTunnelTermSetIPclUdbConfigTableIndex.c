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
* @file tgfTunnelTermSetIPclUdbConfigTableIndex.c
*
* @brief SIP5 tunnel term IPclUdbConfigTableIndex testing
*
* @version   6
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
#include <common/tgfVntGen.h>
#include <common/tgfTunnelGen.h>

#include <pcl/prvTgfPclUdb.h>

/* default tunnel term entry index (first rule) */
static GT_U32        prvTgfTtiRuleLookup0Index;
/* default tunnel term entry index (second rule) */
static GT_U32        prvTgfTtiRuleLookup1Index;

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS      0

/* port index to receive traffic */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS   1

/* user defined ethertype */
#define PRV_TGF_UDE_TAG_CNS         0x8888

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;


/******************************* UDE packet ***********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x11}                 /* saMac */
};

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
};


/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketUsedDefinedEtherTypePart = {PRV_TGF_UDE_TAG_CNS};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfUdePacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketUsedDefinedEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of UDE packet */
#define PRV_TGF_UDE_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Length of UDE packet with CRC */
#define PRV_TGF_UDE_PACKET_CRC_LEN_CNS  PRV_TGF_UDE_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* UDE packet to send */
static TGF_PACKET_STC prvTgfUdePacketInfo =
{
    PRV_TGF_UDE_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfUdePacketPartArray) / sizeof(prvTgfUdePacketPartArray[0]), /* numOfParts */
    prvTgfUdePacketPartArray                                                /* partsArray */
};
/******************************************************************************/


/**
* @internal prvTgfTunnelTermSetIPclUdbConfigTableIndexSetPclConfig function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_UDE_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set rule index to 0
*/
static GT_VOID prvTgfTunnelTermSetIPclUdbConfigTableIndexSetPclConfig
(
    GT_VOID
)
{
    GT_STATUS  rc         = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_UDB */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC: configure 16 User Defined Bytes (UDBs): */
    /* AUTODOC:   format INGRESS_EXT_UDB */
    /* AUTODOC:   packet type UDE, idx [0..15] */
    /* AUTODOC:   offsetType OFFSET_L2, offset [0..15] */
    /* AUTODOC: set PCL rule 0: */
    /* AUTODOC:   format INGRESS_EXT_UDB, cmd DROP_HARD */
    /* AUTODOC:   pattern - packet's [0..15] bytes */
    prvTgfPclUdbTestInit(0, PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
                         PRV_TGF_PCL_PACKET_TYPE_UDE_E, PRV_TGF_PCL_OFFSET_L2_E,
                         GT_FALSE, GT_FALSE, &prvTgfUdePacketInfo);

    /* AUTODOC: set the EtherType 0x8888 to identify UDE */
    rc = prvTgfVntCfmEtherTypeSet(PRV_TGF_UDE_TAG_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntCfmEtherTypeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_UDE_TAG_CNS);
}

/**
* @internal prvTgfTunnelTermSetIPclUdbConfigTableIndexSetTtiRule function
* @endinternal
*
* @brief   Set tti rule to match all the packets and modify
*         iPclUdbConfigTableEnable = GT_TRUE;
*         iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E;
* @param[in] isSecondRule             -  0 - first rule; 1 - second rule
*                                       None
*/
static GT_VOID prvTgfTunnelTermSetIPclUdbConfigTableIndexSetTtiRule
(
    IN GT_U32                isSecondRule
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ACTION_2_STC ttiAction2;
    GT_U32                   ruleIndex;

    ruleIndex = isSecondRule
        ? prvTgfTtiRuleLookup1Index : prvTgfTtiRuleLookup0Index;


    /* AUTODOC: clear entry */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", 0);

    /* AUTODOC: add TTI rule with: match all the packets */

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Ether Type Key */
    ttiAction2.iPclUdbConfigTableEnable = GT_TRUE;
    ttiAction2.iPclUdbConfigTableIndex  = isSecondRule ? PRV_TGF_PCL_PACKET_TYPE_UDE_E : PRV_TGF_PCL_PACKET_TYPE_UDE5_E;
    ttiAction2.continueToNextTtiLookup  = isSecondRule ? GT_FALSE : GT_TRUE;

    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_ETH_E, &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* AUTODOC: validate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(ruleIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfTunnelTermSetIPclUdbConfigTableIndexTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:12,
*         Success Criteria:
*         Packet is captured on ports 0, 8, 18, 23
* @param[in] isPclDropTrafficExpected - is pcl must drop traffic expected
*                                       None
*/
static GT_VOID prvTgfTunnelTermSetIPclUdbConfigTableIndexTrafficGenerate
(
    IN GT_BOOL isPclDropTrafficExpected
)
{

    GT_STATUS  rc         = GT_OK;
    GT_U32     portIter   = 0;
    GT_U32     portsCount = prvTgfPortsNum;

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Sending unknown unicast
     */
    PRV_UTF_LOG0_MAC("======= Sending unknown unicast =======\n");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfUdePacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d", prvTgfDevNum);



    PRV_UTF_LOG0_MAC("======= Checking Counters =======\n");

    /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS == portIter) ? 1 : 0),
            /*Tx*/((PRV_TGF_SEND_PORT_IDX_CNS == portIter) ?
                                        1 : GT_TRUE == isPclDropTrafficExpected ? 0 : 1),
            PRV_TGF_UDE_PACKET_LEN_CNS,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

}

/**
* @internal prvTgfTunnelTermSetIPclUdbConfigTableIndexConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfTunnelTermSetIPclUdbConfigTableIndexConfigRestore
(
    GT_VOID
)
{
    GT_STATUS  rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup0Index, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                                        prvTgfTtiRuleLookup0Index, GT_FALSE);
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup1Index, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                                        prvTgfTtiRuleLookup1Index, GT_FALSE);
    prvTgfPclUdeConfigRestore();
}

/* main test func */
GT_VOID tgfTunnelTermSetIPclUdbConfigTableIndexTest
(
    GT_VOID
)
{
    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3);
    prvTgfTtiRuleLookup1Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1, 0);

    prvTgfTunnelTermSetIPclUdbConfigTableIndexSetPclConfig();
    prvTgfTunnelTermSetIPclUdbConfigTableIndexTrafficGenerate(GT_TRUE);

    prvTgfTunnelTermSetIPclUdbConfigTableIndexSetTtiRule(0 /*first rule*/);
    prvTgfTunnelTermSetIPclUdbConfigTableIndexTrafficGenerate(GT_FALSE);

    prvTgfTunnelTermSetIPclUdbConfigTableIndexSetTtiRule(1 /*second rule*/);
    prvTgfTunnelTermSetIPclUdbConfigTableIndexTrafficGenerate(GT_TRUE);

    prvTgfTunnelTermSetIPclUdbConfigTableIndexConfigRestore();
}






