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
* @file tgfTunnelTermSetIPclOverrideConfigTableIndex.c
*
* @brief SIP5 tunnel term IPclOverrideConfigIndex testing
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfVntGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfBridgeGen.h>

#include <pcl/prvTgfPclUdb.h>

/* default tunnel term entry index (first rule) */
static GT_U32        prvTgfTtiRuleLookup0Index;
/* default tunnel term entry index (second rule) */
static GT_U32        prvTgfTtiRuleLookup1Index;

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS      0

/* port index to receive traffic */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS   3

/* user defined ethertype */
#define PRV_TGF_UDE_TAG_CNS         0x8888

/* default vlan id */
#define PRV_TGF_DEFAULT_VLAN_ID_CNS   1

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS        3

/* number of pcl rules */
#define PRV_TGF_NUM_OF_PCL_RULES_CNS    3

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/* indexes of PCL Rules - 255, 511, 383 */
static GT_U16 prvTgfPclIndex[PRV_TGF_NUM_OF_PCL_RULES_CNS] = {0xFF, 0x1FF, 0x17F};

static GT_U16 prvTgfPclLookupId[PRV_TGF_NUM_OF_PCL_RULES_CNS] = {0,1,2};

/******************************* UDE packet ***********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x11}                 /* saMac */
};

/* Data of packet */
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


/* Payload part */
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




/******************************************************************************/
static GT_VOID  prvTgfInitPclLookup
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum

)
{
    GT_STATUS  rc  = GT_OK;

    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ethOthKey  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv4Key    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ipv6Key    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: init ipcl lookup for port 0 */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              lookupNum,
                              ethOthKey, ipv4Key, ipv6Key);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d",
                                                prvTgfDevNum, lookupNum);
}

/**
* @internal prvTgfTunnelTermSetIPclOverrideConfigTableIndexSetPclConfig function
* @endinternal
*
* @brief   Set test configuration:
*         - set packet type to PRV_TGF_PCL_PACKET_TYPE_UDE_E
*         - set key type to PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
*         - set UDB offset type to PRV_TGF_PCL_OFFSET_L2_E
*         - set rule index to 0
*/
static GT_VOID prvTgfTunnelTermSetIPclOverrideConfigTableIndexSetPclConfig
(
    GT_VOID
)
{
    GT_STATUS  rc  = GT_OK;

    PRV_TGF_PCL_RULE_FORMAT_UNT       pattern;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_ACTION_STC            action;
    GT_U32  finalIndex;

    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    GT_U32                            ruleIndex  = 0;

    GT_U16                            vlanId  = PRV_TGF_DEFAULT_VLAN_ID_CNS;

    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    /* AUTODOC: ini tti rules lookup 0/1 base tcam indexes */
    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3);
    prvTgfTtiRuleLookup1Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1, 0);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", 0);

    /* AUTODOC: create VLAN 6 with untagged port [3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS,
                    prvTgfPortsArray + PRV_TGF_RECEIVE_PORT_IDX_CNS,
                    NULL, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);


    /* AUTODOC: set the EtherType 0x8888 to identify UDE */
    rc = prvTgfVntCfmEtherTypeSet(PRV_TGF_UDE_TAG_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfVntCfmEtherTypeSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_UDE_TAG_CNS);

    /* AUTODOC: init ipcl lookup0 for port 0 */
    prvTgfInitPclLookup(CPSS_PCL_LOOKUP_NUMBER_0_E);
    /* AUTODOC: init ipcl lookup1 for port 0 */
    prvTgfInitPclLookup(CPSS_PCL_LOOKUP_NUMBER_1_E);
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: init ipcl lookup2 for port 0 */
        prvTgfInitPclLookup(CPSS_PCL_LOOKUP_NUMBER_2_E);
    }

    /* AUTODOC: set the iPcl rules */
    for(ruleIndex = 0; ruleIndex < PRV_TGF_NUM_OF_PCL_RULES_CNS; ruleIndex++)
    {
        if ((prvTgfPclLookupId[ruleIndex] >= 2) &&
            PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
        {
            continue;
        }
        cpssOsMemSet(&mask,    0, sizeof(mask));
        cpssOsMemSet(&pattern, 0, sizeof(pattern));
        cpssOsMemSet(&action,  0, sizeof(action));

        switch(ruleIndex)
        {
            case 0:
                mask.ruleStdNotIp.common.vid = 0xF;
                pattern.ruleStdNotIp.common.vid = vlanId; /* check vlan id 1 */

                action.vlan.vlanId = ++vlanId; /* set vlan id 2 */
                if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
                {
                    action.vlan.vlanId = ++vlanId; /* set vlan id 3 */
                }
                action.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
                break;
            case 1:
                if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
                {
                    mask.ruleStdNotIp.common.vid = 0xF;
                    pattern.ruleStdNotIp.common.vid = vlanId; /* check vlan id 2 */
                    action.vlan.vlanId = ++vlanId; /* set vlan id 3 */
                    action.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;

                    break;
                }
            case 2:
                mask.ruleStdNotIp.common.vid = 0xF;
                pattern.ruleStdNotIp.common.vid = vlanId; /* check vlan id 3 */
                action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E; /* drop the packet */

                break;
            default:
                /* do nothing */
                break;
        }

        finalIndex = prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(prvTgfPclLookupId[ruleIndex],prvTgfPclIndex[ruleIndex]);

        rc = prvTgfPclRuleSet(ruleFormat, finalIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d", prvTgfDevNum);
    }

}

/**
* @internal prvTgfTunnelTermSetIPclOverrideConfigTableIndexSetTtiRule function
* @endinternal
*
* @brief   Set tti rule to match all the packets and modify pcl 2 override index
*
* @param[in] isSecondRule             -  0 - first rule; 1 - second rule
*                                       None
*/
static GT_VOID prvTgfTunnelTermSetIPclOverrideConfigTableIndexSetTtiRule
(
    IN GT_U32                isSecondRule
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ACTION_2_STC ttiAction2;
    GT_U32                   ruleIndex;

    ruleIndex = isSecondRule ? prvTgfTtiRuleLookup1Index : prvTgfTtiRuleLookup0Index;

    /* AUTODOC: clear entry */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: add TTI rule with: match all the packets */

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Ether Type Key */
    if(0 == isSecondRule)
    {
        /* first rule */
        /* using default values */
        ttiAction2.pcl0_1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        ttiAction2.pcl1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        ttiAction2.continueToNextTtiLookup  = GT_TRUE;
    }
    else
    {
        /* second rule */
        /* override values */
        if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            ttiAction2.pcl1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        }
        else
        {
            ttiAction2.pcl0_1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        }
        ttiAction2.continueToNextTtiLookup  = GT_FALSE;
    }


    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_ETH_E, &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* AUTODOC: validate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(ruleIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfTunnelTermSetIPclOverrideConfigTableIndexTrafficGenerate function
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
static GT_VOID prvTgfTunnelTermSetIPclOverrideConfigTableIndexTrafficGenerate
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
    rc = prvTgfEthCountersCheck(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        /*Rx*/ 0,
        /*Tx*/ (isPclDropTrafficExpected ? 0 : 1),
        PRV_TGF_UDE_PACKET_LEN_CNS,
        prvTgfBurstCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
                        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

}

/**
* @internal prvTgfTunnelTermSetIPclOverrideConfigTableIndexConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfTunnelTermSetIPclOverrideConfigTableIndexConfigRestore
(
    GT_VOID
)
{
    GT_STATUS  rc = GT_OK;
    GT_U32     ruleIndex = 0;
    GT_U32  finalIndex;

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

    for(ruleIndex = 0; ruleIndex < PRV_TGF_NUM_OF_PCL_RULES_CNS; ruleIndex++)
    {
        if ((prvTgfPclLookupId[ruleIndex] >= 2) &&
            PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
        {
            continue;
        }
        finalIndex = prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(prvTgfPclLookupId[ruleIndex],prvTgfPclIndex[ruleIndex]);

        /* AUTODOC: invalidate PCL rule */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                            finalIndex, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");
    }

    prvTgfPclUdeConfigRestore();
}


/* main test func */
GT_VOID tgfTunnelTermSetIPclOverrideConfigTableIndexTest
(
    GT_VOID
)
{
    /* AUTODOC: Test functionality of TTI iPclOverrideConfigTableIndex:

        0. set base configuration:

            ini tti rules lookup 0/1 base tcam indexes
            enable TTI lookup for port 0, key TTI_KEY_ETH
            create VLAN 6 with untagged port [3]
            set the EtherType 0x8888 to identify UDE
            init ipcl lookup CPSS_PCL_LOOKUP_NUMBER_0_E for port 0
            init ipcl lookup CPSS_PCL_LOOKUP_NUMBER_1_E for port 0
            init ipcl lookup CPSS_PCL_LOOKUP_NUMBER_2_E for port 0

        1. configure iPcl to drop traffic:

            set the iPCL Rules:
                 rule 1:  match vlan id 1 - set vlan id 2
                 rule 2:  match vlan id 2 - set vlan id 3
                 rule 3:  match vlan id 3 - drop the packet */
    prvTgfTunnelTermSetIPclOverrideConfigTableIndexSetPclConfig();

    /* AUTODOC:

        2. send traffic and check that it is dropped:

            CPSS_PCL_LOOKUP_NUMBER_0_E matches rule 1: match vlan id 1 - set vlan id 2
            CPSS_PCL_LOOKUP_NUMBER_1_E matches rule 2: match vlan id 2 - set vlan id 3
            CPSS_PCL_LOOKUP_NUMBER_2_E matches rule 3: match vlan id 3 - drop the packet

            ---> The packet dropped by pcl lookup 2.  */

    prvTgfTunnelTermSetIPclOverrideConfigTableIndexTrafficGenerate(GT_TRUE);


    /* AUTODOC:
        3. setup tti rule to not modify ingress pcl config index:

            first rule - using default values
            add TTI rule with: match all the packets

            ttiAction2.pcl1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
            ttiAction2.continueToNextTtiLookup  = GT_TRUE; */
    prvTgfTunnelTermSetIPclOverrideConfigTableIndexSetTtiRule(0 /*first rule*/);


    /* AUTODOC:
        4. send traffic and check that it is dropped - no pcl config changes:

            CPSS_PCL_LOOKUP_NUMBER_0_E matches rule 1: match vlan id 1 - set vlan id 2
            CPSS_PCL_LOOKUP_NUMBER_1_E matches rule 2: match vlan id 2 - set vlan id 3
            CPSS_PCL_LOOKUP_NUMBER_2_E config index was not changed by second tti rule
            CPSS_PCL_LOOKUP_NUMBER_2_E matches rule 3: match vlan id 3 - drop the packet

            ---> The packet dropped by pcl lookup 2.  */
    prvTgfTunnelTermSetIPclOverrideConfigTableIndexTrafficGenerate(GT_TRUE);


    /* AUTODOC:
        5. check dual lookup: setup second tti rule to modify pcl config index:

            second rule - override values
            add TTI rule with: match all the packets
            ttiAction2.pcl1OverrideConfigIndex  = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            ttiAction2.continueToNextTtiLookup  = GT_FALSE; */
    prvTgfTunnelTermSetIPclOverrideConfigTableIndexSetTtiRule(1 /*second rule*/);

    /* AUTODOC:
        6. send traffic and check that it is not dropped by pcl rule:

            CPSS_PCL_LOOKUP_NUMBER_0_E matches rule 1: match vlan id 1 - set vlan id 2
            CPSS_PCL_LOOKUP_NUMBER_1_E matches rule 2: match vlan id 2 - set vlan id 3
            CPSS_PCL_LOOKUP_NUMBER_2_E config index was changed by second tti rule
            CPSS_PCL_LOOKUP_NUMBER_2_E disabled - so no drop the packet

            ---> The packet forwarded with modified vid 3 to egress port */
    prvTgfTunnelTermSetIPclOverrideConfigTableIndexTrafficGenerate(GT_FALSE);


    /* AUTODOC: 7. restore configuration */
    prvTgfTunnelTermSetIPclOverrideConfigTableIndexConfigRestore();
}






