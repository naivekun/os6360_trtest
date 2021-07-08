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
* @file prvTgfTunnelTermTtiFullDb.c
*
* @brief Verify the functionality of Tunnel Term TTI TCAM Filling
*
* @version   5
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>
#include <common/tgfTcamGen.h>

#include <tunnel/prvTgfTunnelTermTtiFullDb.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0

/* port index to receive  traffic n */
#define PRV_TGF_EGR_PORT_IDX0_CNS  2
#define PRV_TGF_EGR_PORT_IDX_CNS   3

/* number of ports in VLans */
#define PRV_TGF_RX_SIDE_PORTS_COUNT_CNS 2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* send VLAN */
#define PRV_TGF_VLANID_5_CNS          5
/* receive VLAN id */
#define PRV_TGF_VLANID_6_CNS          6

/* the TTI Rule index */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0,0)

/* the TTI Rule number of indexes allowed -  each index specify one rule of size 10 */
#define PRV_TGF_TTI0_NUM_OF_INDEXES_CNS (prvTgfTcamTtiNumOfIndexsGet(0))

/* TTI floor 1 */
#define PRV_TGF_TTI_FLOOR1_CNS      1

/* TCAM group */
#define GROUP_1                     1

/* TCAM hit 0 */
#define HIT_NUM_0                   0

/* TCAM floor info saved for restore */
static PRV_TGF_TCAM_BLOCK_INFO_STC saveFloorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];

/* number of packets to send */
static GT_U32   prvTgfBurstCount = 0;

/* TTI Action, Pattern and Mask */
static PRV_TGF_TTI_ACTION_STC   ttiAction;
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;

/* default value for restore */
static GT_BOOL                             firstCallToTtiConfig=GT_TRUE;
static PRV_TGF_TTI_OFFSET_TYPE_ENT         defaultOffsetTypeUdb0;
static GT_U8                               defaultOffsetUdb0;
static PRV_TGF_TTI_OFFSET_TYPE_ENT         defaultOffsetTypeUdb1;
static GT_U8                               defaultOffsetUdb1;
static PRV_TGF_TTI_KEY_SIZE_ENT            defaultUdbKeySize;

/* default the TTI lookup for PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E at the port 0 */
static GT_BOOL                  defaultTtiPortLookupEnableGet;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketEthernetVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet Ethernet */
static TGF_PACKET_PART_STC prvTgfPacketEthernetPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketEthernetVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET Ethernet to send */
static TGF_PACKET_STC prvTgfPacketEthernetInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketEthernetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthernetPartArray                                        /* partsArray */
};

static GT_U32                   packetSize=0;
static TGF_PACKET_PART_STC      *packetPartsPtr;
static GT_U32                   partsCount=0;

static GT_U32                   ruleIndexIncr = 0;
static PRV_TGF_TTI_RULE_TYPE_ENT ruleType;
static PRV_TGF_TTI_KEY_SIZE_ENT keySize;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermTtiFullDbBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID prvTgfTunnelTermTtiFullDbBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;
    GT_U8     tagArray[] = {1, 1, 1, 1};

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                           NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfTunnelTermTtiFullDbTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID prvTgfTunnelTermTtiFullDbTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc                = GT_FAIL;
    GT_U32    i                 = 0;
    GT_U32    maxRuleIndexGet   = 0;/* max legal rule index for Lookup 0*/


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    if(firstCallToTtiConfig==GT_TRUE)
    {
        /* AUTODOC: save UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: udbIndex 0 */
        rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum,
                                         PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                         0,
                                         &defaultOffsetTypeUdb0,
                                         &defaultOffsetUdb0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);

         /* AUTODOC: save UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: udbIndex 1 */
        rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum,
                                         PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                         1,
                                         &defaultOffsetTypeUdb1,
                                         &defaultOffsetUdb1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);


        /* AUTODOC: save key size keySize for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E */
        rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,&defaultUdbKeySize);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

        /* AUTODOC: save the TTI lookup for PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E at the port 0 */
        rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, &defaultTtiPortLookupEnableGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

        firstCallToTtiConfig=GT_FALSE;
    }

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: anchor Metadata, offset 0 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     0,
                                     PRV_TGF_TTI_OFFSET_L2_E,
                                     2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     1,
                                     PRV_TGF_TTI_OFFSET_L2_E,
                                     3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set key size keySize for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, keySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: configure TTI action to change packet's vid to 6 */
    ttiAction.redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.tag0VlanCmd     = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId      = PRV_TGF_VLANID_6_CNS;

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* get the max legal index for Lookup 0 */
    maxRuleIndexGet = PRV_TGF_TTI0_INDEX_CNS + PRV_TGF_TTI0_NUM_OF_INDEXES_CNS;
    for (i = PRV_TGF_TTI0_INDEX_CNS; i < maxRuleIndexGet; i += ruleIndexIncr)
    {
        ttiPattern.udbArray[0] = (GT_U8) (i >> 8);
        ttiMask.udbArray[0]    = 0xFF;

        ttiPattern.udbArray[1] = (GT_U8)(i & 0xFF);
        ttiMask.udbArray[1]    = 0xFF;

        /*PRV_UTF_LOG1_MAC("===== Adding TTI rule %d: =====\n", i);*/

        /* AUTODOC: add TTI rule with ruleType on port 0 VLAN 5 with action: set vid to 6 */
        rc = prvTgfTtiRuleSet(i, ruleType, &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
    }
}

/**
* @internal prvTgfTunnelTermTtiFullDbSendTraffic function
* @endinternal
*
* @brief   Send Traffic
*/
static GT_VOID prvTgfTunnelTermTtiFullDbSendTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_FAIL;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_BOOL                         isOk;

    TGF_VFD_INFO_STC                vfdArray[1];

    /* init burst count value*/
    /* send number of packets according to max indexes allowed for lookup 0,
       according to the rule size we are checking */
    prvTgfBurstCount = (PRV_TGF_TTI0_NUM_OF_INDEXES_CNS/ruleIndexIncr);

    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray[0]));
    cpssOsMemSet((GT_VOID*) &expectedCntrs, 0, sizeof(expectedCntrs));

    vfdArray[0].mode          = TGF_VFD_MODE_INCREMENT_VALUE_E; /* increment by value --> see parameter incValue */

    vfdArray[0].incValue      = ruleIndexIncr; /* for increment_value,decrement_value  it is the value for inc/dec */
    vfdArray[0].offset        = 2; /* offset in the packet to override */

    vfdArray[0].patternPtr[0] = (GT_U8)(PRV_TGF_TTI0_INDEX_CNS >> 8) ;
    vfdArray[0].patternPtr[1] = (GT_U8)(PRV_TGF_TTI0_INDEX_CNS & 0xFF) ;

    vfdArray[0].cycleCount = 2; /* two bytes used in vfdArray[0].patternPtr */


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters
     */

    /*  Reset all Ethernet port's counters and clear capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketEthernetInfo, prvTgfBurstCount, 1, vfdArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth\n");

    /* AUTODOC: calculate packet size */
    partsCount = sizeof(prvTgfPacketEthernetPartArray) / sizeof(TGF_PACKET_PART_STC);
    packetPartsPtr = prvTgfPacketEthernetPartArray;
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth\n");

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:
                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
                break;
            case PRV_TGF_EGR_PORT_IDX0_CNS: /* expect flood to vlan 6 since there is no bridge configuration */
            case PRV_TGF_EGR_PORT_IDX_CNS:
                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                break;
            default:
                 /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];


        UTF_VERIFY_EQUAL1_STRING_MAC(isOk, GT_TRUE, "Got another counters values on port [%d]",
                                                                prvTgfPortsArray[portIter]);

        /* print expected values if bug */
        if (isOk != GT_TRUE) {

            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }
}


/**
* @internal prvTgfTunnelTermTtiFullDbConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
static GT_VOID prvTgfTunnelTermTtiFullDbConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;
    GT_U32    i = 0;

     /* AUTODOC: Remove VLAN 5 members */
    for (i = 0; i < PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_5_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_5_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: Remove VLAN 6 members */
    for (i = PRV_TGF_RX_SIDE_PORTS_COUNT_CNS; i < PRV_TGF_PORT_COUNT_CNS; i++)
    {
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                       prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_VLANID_6_CNS,
                                     prvTgfPortsArray[i]);
    }

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_6_CNS);

    /* AUTODOC: restore the TTI lookup for PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_FALSE/*defaultTtiPortLookupEnableGet*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: anchor Metadata, offset 0 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     0,
                                     defaultOffsetTypeUdb0,
                                     defaultOffsetUdb0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: restore UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: anchor Metadata, offset 0 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                     PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                     1,
                                     defaultOffsetTypeUdb1,
                                     defaultOffsetUdb1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: restore key size keySize for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, defaultUdbKeySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    if(prvTgfTcamFloorsNumGet() <= 3)
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(PRV_TGF_TTI_FLOOR1_CNS, &saveFloorInfoArr[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
            PRV_TGF_TTI_FLOOR1_CNS);
    }

    firstCallToTtiConfig=GT_TRUE;
}

/**
* @internal prvTgfTunnelTermTtiFullDb function
* @endinternal
*
* @brief   Test of Tunnel Term TTI Dsa Metadata (main func)
*/
GT_VOID prvTgfTunnelTermTtiFullDb
(
    GT_VOID
)
{
    GT_STATUS rc;

    if(prvTgfTcamFloorsNumGet() <= 3)
    {
        GT_U32 ii;
        PRV_TGF_TCAM_BLOCK_INFO_STC testFloorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];

        /* AUTODOC: Save TCAM floor info for restore */
        rc = prvTgfTcamIndexRangeHitNumAndGroupGet(PRV_TGF_TTI_FLOOR1_CNS, &saveFloorInfoArr[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupGet: %d", PRV_TGF_TTI_FLOOR1_CNS);

        /* AUTODOC: Floor index 1: use all blocks for HIT_NUM_0 */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            testFloorInfoArr[ii].hitNum = HIT_NUM_0;
            testFloorInfoArr[ii].group  = GROUP_1;
        }
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(PRV_TGF_TTI_FLOOR1_CNS, &testFloorInfoArr[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
            PRV_TGF_TTI_FLOOR1_CNS);
    }

    /* check 10/20/30 udb keys */
    for(keySize =  PRV_TGF_TTI_KEY_SIZE_10_B_E;
        keySize <= PRV_TGF_TTI_KEY_SIZE_30_B_E;
        keySize++)
    {
        /* AUTODOC: set rule type and tti index increment according to key type */
        switch (keySize)
        {
            case PRV_TGF_TTI_KEY_SIZE_10_B_E:
                PRV_UTF_LOG0_MAC("===== Starting with rule size 10 =====\n");
                ruleIndexIncr = 1;
                ruleType = PRV_TGF_TTI_RULE_UDB_10_E;
                break;
            case PRV_TGF_TTI_KEY_SIZE_20_B_E:
                PRV_UTF_LOG0_MAC("===== Starting with rule size 20 =====\n");
                ruleIndexIncr = 2;
                ruleType = PRV_TGF_TTI_RULE_UDB_20_E;
                break;
            case PRV_TGF_TTI_KEY_SIZE_30_B_E:
                PRV_UTF_LOG0_MAC("===== Starting with rule size 30 =====\n");
                ruleIndexIncr = 3;
                ruleType = PRV_TGF_TTI_RULE_UDB_30_E;
                break;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, GT_FALSE, "bad key size given: %d",
                                                            PRV_TGF_TTI_KEY_SIZE_10_B_E);
        }


        /* AUTODOC: create VLAN 5 with tagged ports [0,1] */
        /* AUTODOC: create VLAN 6 with tagged ports [2,3] */
        prvTgfTunnelTermTtiFullDbBridgeConfigSet();

        /* AUTODOC: add TTI rules with rule size PRV_TGF_TTI_RULE_UDB_10_E
            for key size with command: FORWARD */

        prvTgfTunnelTermTtiFullDbTtiConfigSet();
        prvTgfTunnelTermTtiFullDbSendTraffic();

        {
            GT_U32    i  = 0;
            GT_U32    maxRuleIndexGet   = 0;/* max legal rule index for Lookup 0*/
            rc = GT_FAIL;

            /* get the max legal index for Lookup 0 */
            maxRuleIndexGet = PRV_TGF_TTI0_INDEX_CNS + PRV_TGF_TTI0_NUM_OF_INDEXES_CNS;
            for (i = PRV_TGF_TTI0_INDEX_CNS; i < maxRuleIndexGet; i += ruleIndexIncr)
            {
                /* AUTODOC: invalidate TTI rule */
                rc = prvTgfTtiRuleValidStatusSet(i, GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");
            }
        }
    }
    /*------------------------------------------------------------*/
    /* AUTODOC: test configuration restore */
    prvTgfTunnelTermTtiFullDbConfigRestore();
}


