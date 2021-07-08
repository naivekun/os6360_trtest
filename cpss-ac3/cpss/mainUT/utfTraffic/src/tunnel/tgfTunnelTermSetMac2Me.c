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
* @file tgfTunnelTermSetMac2Me.c
*
* @brief SIP5 tunnel term setMac2Me testing
*
* @version   4
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <pcl/prvTgfPclBc2.h>
#include <tunnel/tgfTunnelTermSetMac2Me.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS    2

/* lookup1 rule index */
#define PRV_TGF_LOOKUP1_RULE_IDX_CNS    prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(1/*lookupId*/,5)

/* original VLAN Id */
#define PRV_TGF_ORIGINAL_VLAN_ID_CNS    3

/* lookup1 VLAN Id */
#define PRV_TGF_LOOKUP1_VLAN_ID_CNS     7

static struct{

    GT_U32 isValid;
    GT_U32 ttiRuleIndex;
    GT_U32 mac2me;

}lookupInfoStc[PRV_TGF_MAX_LOOKUPS_NUM_CNS];

/********************************************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketEthOth1L2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthOth1VlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,        /* etherType */
    0, 0, PRV_TGF_ORIGINAL_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOth1EthertypePart =
{
    0x3333
};

/* DATA of packet */
static GT_U8 prvTgfPacketEthOth1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketEthOth1PayloadPart = {
    sizeof(prvTgfPacketEthOth1PayloadDataArr),            /* dataLength */
    prvTgfPacketEthOth1PayloadDataArr                     /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketEthOth1PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketEthOth1L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketEthOth1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketEthOth1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketEthOth1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketEthOth1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketEthOth1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthOth1PartArray                                        /* partsArray */
};

/* L2 part of packet2 */
static TGF_PACKET_L2_STC prvTgfPacketEthOth2L2Part =
{
    {0x00, 0xA9, 0x87, 0x65, 0x43, 0x21},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

/* PARTS of packet2 */
static TGF_PACKET_PART_STC prvTgfPacketEthOth2PartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketEthOth2L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketEthOth1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketEthOth1PayloadPart}
};

/* PACKET2 to send */
static TGF_PACKET_STC prvTgfPacketEthOth2Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketEthOth1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketEthOth2PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthOth2PartArray                                        /* partsArray */
};


/**
* @internal tgfTunnelTermSetMac2MeSetMac2MeWithTtiRule function
* @endinternal
*
* @brief   Setup TTI rule to set mac2me for given mac address
*
* @param[in] lookupNum                - lookup number
* @param[in] mac2me                   - mac to me value to set to the rule
*                                       None
*/
static GT_VOID tgfTunnelTermSetMac2MeSetMac2MeWithTtiRule
(
    GT_U8  lookupNum,
    GT_BOOL mac2me
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ACTION_2_STC ttiAction2;

    /* AUTODOC:   add TTI rule with: */
        /* AUTODOC:   key TTI_KEY_ETH, cmd FORWARD, redirectCmd NO_REDIRECT */

    /* AUTODOC: clear entry */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for Ether Type Key */
    ttiAction2.command    = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2.setMacToMe = mac2me;

    if(lookupNum != (PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1) )
    {
        ttiAction2.continueToNextTtiLookup = GT_TRUE;
    }

    /* AUTODOC: set TTI Pattern - macDa*/
    cpssOsMemCpy((GT_VOID*)ttiPattern.eth.common.mac.arEther,
                 (GT_VOID*)prvTgfPacketEthOth1L2Part.daMac,
                 sizeof(ttiPattern.eth.common.mac.arEther));

    /* AUTODOC: configure TTI rule mask - macDa */
    cpssOsMemSet((GT_VOID*)ttiMask.eth.common.mac.arEther, 0xFF,
                 sizeof(ttiMask.eth.common.mac.arEther));

    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRule2Set(lookupInfoStc[lookupNum].ttiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                                      &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* AUTODOC: validate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(lookupInfoStc[lookupNum].ttiRuleIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                                    prvTgfDevNum, GT_TRUE);
}


/**
* @internal tgfTunnelTermSetMac2MeTest function
* @endinternal
*
* @brief   Main test function
*/
GT_VOID tgfTunnelTermSetMac2MeTest
(
    GT_VOID
)
{
    GT_U8 lookupNum;

    /* AUTODOC: set base configuration */
    prvTgfPclBc2Mac2meSetBaseConfiguration();

    /* AUTODOC: check mac2me using pcl rule */
    prvTgfPclBc2Mac2meCheckMac2MeWithPclRule();


    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS; lookupNum++)
    {
        PRV_UTF_LOG1_MAC("\n============================ Process lookup %d =======================\n", lookupNum);

        lookupInfoStc[lookupNum].isValid = 1;
        lookupInfoStc[lookupNum].ttiRuleIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(lookupNum, 0);
        lookupInfoStc[lookupNum].mac2me = GT_TRUE;

        /* AUTODOC: set mac2me using tti rule */
        tgfTunnelTermSetMac2MeSetMac2MeWithTtiRule(lookupNum, lookupInfoStc[lookupNum].mac2me);

        PRV_UTF_LOG0_MAC("\n============== Check matched packet  ================\n");
        /* AUTODOC: send matched packet */
        prvTgfPclBc2VidTrafficGenerate(&prvTgfPacketEthOth1Info);

        /* AUTODOC: check matched packet */
        prvTgfPclBc2TrafficEgressVidCheck(
            PRV_TGF_RECEIVE_PORT_IDX_CNS,
            PRV_TGF_LOOKUP1_VLAN_ID_CNS,
            GT_FALSE /*checkMsb*/);

        PRV_UTF_LOG0_MAC("\n============== Check unmatched packet  ================\n");
        /* AUTODOC: send unmatched packet */
        prvTgfPclBc2VidTrafficGenerate(&prvTgfPacketEthOth2Info);

        /* AUTODOC: check unmatched packet */
        prvTgfPclBc2TrafficEgressVidCheck(
            PRV_TGF_RECEIVE_PORT_IDX_CNS,
            PRV_TGF_ORIGINAL_VLAN_ID_CNS,
            GT_FALSE /*checkMsb*/);

        /* AUTODOC: update the rule to be zero mac to me field */
        tgfTunnelTermSetMac2MeSetMac2MeWithTtiRule(lookupNum, GT_FALSE);
    }


}

/**
* @internal tgfTunnelTermSetMac2MeTestRestore function
* @endinternal
*
* @brief   Restore after test configuration
*/
GT_VOID tgfTunnelTermSetMac2MeTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    lookupNum;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    for(lookupNum = 0; lookupNum < sizeof(lookupInfoStc)/sizeof(lookupInfoStc[0]); lookupNum++)
    {
        if (lookupInfoStc[lookupNum].isValid)
        {
            /* AUTODOC: invalidate TTI rules */
            rc = prvTgfTtiRuleValidStatusSet(lookupInfoStc[lookupNum].ttiRuleIndex, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d",
                                                                prvTgfDevNum, GT_FALSE);
            lookupInfoStc[lookupNum].isValid = 0;
        }
    }


    /* AUTODOC: disables configuration table and port configurations */
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_NUMBER_1_E);

    /* AUTODOC: invalidate PCL rule for Lookup1 */
    rc = prvTgfPclRuleValidStatusSet( CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_LOOKUP1_RULE_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN Entry VID == 7 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_LOOKUP1_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: invalidate VLAN Entry VID == 3 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ORIGINAL_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", prvTgfDevNum);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}



