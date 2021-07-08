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
* @file prvTgfTunnelTermSourceEportAssignment.c
*
* @brief Verify the functionality of TTI action Source Eport Assignment field
* for parallel lookup
*
* @version   2
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

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS          6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_1_PORT_IDX_CNS      1

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_2_PORT_IDX_CNS      2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* new source eport  */
#define PRV_TGF_NEW_SRC_EPORT_BASE_CNS   600

static GT_U32  sourceEportArray[PRV_TGF_MAX_LOOKUPS_NUM_CNS];

/* this is the first call to the function - need to do global configuration */
static GT_BOOL firstCall = GT_TRUE;

static struct{

    GT_U32 isValid;
    GT_U32 ttiRuleIndex;
    GT_BOOL sourceEPortAssignmentEnable;  /* source ePort assignment enable for lookup  */
    GT_U32 sourceEport;      /* source ePort value for lookup  */

}lookupInfoStc[PRV_TGF_MAX_LOOKUPS_NUM_CNS];


/* original tcam segment mode value (for configuration restore) */
static PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT tcamSegmentModeOrig;


/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/******************************* Test packet **********************************/

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x05},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x15}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

static TGF_PACKET_MPLS_STC prvTgfPacketMplsLabelPart = {
    0x1000,             /* label */
    0,                  /* CoS */
    1,                  /* S */
    0xff                /* TTL */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of basic packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLabelPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfTunnelTermSourceEportAssignmentBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID tgfTunnelTermSourceEportAssignmentBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 1,
                                           NULL, NULL, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal tgfTunnelTermSourceEportAssignmentTtiConfigSet function
* @endinternal
*
* @brief   set TTI Basic configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID tgfTunnelTermSourceEportAssignmentTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_ENT   tcamSegmentMode;

    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_MPLS_E */
    /* AUTODOC: enable the TTI lookup for MPLS at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                        PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: save TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &tcamSegmentModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeGet");

    tcamSegmentMode =  PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE ?
                                                PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_FOUR_TCAM_E :
                                                PRV_TGF_TTI_KEY_TCAM_SEGMENT_MODE_TWO_TCAM_E ;

    /* AUTODOC: set TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, tcamSegmentMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");
}

/**
* @internal tgfTunnelTermSourceEportAssignmentTtiConfigPerLookupSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID tgfTunnelTermSourceEportAssignmentTtiConfigPerLookupSet
(
    GT_U8 lookupNum
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction,  0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Rule =======\n");

    /* AUTODOC: set the TTI Rule Action for all the packets */
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;

    switch (lookupNum)
    {
        case 0:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_1_PORT_IDX_CNS];
            break;
        case 1:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_2_PORT_IDX_CNS];
            break;
        case 2:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_1_PORT_IDX_CNS];
            break;
        case 3:
            ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_2_PORT_IDX_CNS];
            break;
        default:
            return;
    }

    ttiAction.sourceEPortAssignmentEnable = lookupInfoStc[lookupNum].sourceEPortAssignmentEnable;
    ttiAction.sourceEPort                 = lookupInfoStc[lookupNum].sourceEport;

    ttiAction.ingressPipeBypass = GT_FALSE;
    ttiAction.bridgeBypass = GT_TRUE;

    ttiAction.tag0VlanCmd       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId        = PRV_TGF_EGR_VLANID_CNS;

    if(lookupNum != (PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS-1) )
    {
        ttiAction.continueToNextTtiLookup = GT_TRUE;
    }

    /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with action: sourceEport Assignment enable */
    rc = prvTgfTtiRule2Set(lookupInfoStc[lookupNum].ttiRuleIndex,
                            PRV_TGF_TTI_KEY_MPLS_E, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal tgfTunnelTermSourceEportAssignmentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectTraffic            - GT_TRUE: expect the traffic to be forwarded
*                                      GT_FALSE: expect the traffic to be dropped
* @param[in] lookupNum                - the lookup number we expect to get a match on
*                                       None
*/
static GT_VOID tgfTunnelTermSourceEportAssignmentTrafficGenerate
(
    IN GT_BOOL  expectTraffic,
    GT_U32      lookupNum
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          portIterToCheck = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          numOfExpectedRxPackets;
    PRV_TGF_MAC_ENTRY_KEY_STC       macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_2_PORT_IDX_CNS];
    portIterToCheck = PRV_TGF_EGR_2_PORT_IDX_CNS;

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    numOfExpectedRxPackets = expectTraffic == GT_FALSE ? 0 : prvTgfBurstCount;

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:05, SA=00:00:00:00:00:15 */
    /* AUTODOC:   VID=5, Ethertype=0x8847, MPLS Label=0x1000 */
    /* AUTODOC:   Passenger DA=00:00:00:00:34:02, SA=00:00:00:00:00:12 */

    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: read counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth\n");

        if(portIterToCheck == portIter)
        {
            /* AUTODOC: verify to get Ethernet packet on port 2 with: */
            UTF_VERIFY_EQUAL1_STRING_MAC(portCntrs.goodPktsRcv.l[0], numOfExpectedRxPackets,
                "there is no traffic on expected port, packet Ether type: 0x%X\n",
                                                prvTgfPacketEtherTypePart.etherType);
        }
    }

   /* get FDB entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_EGR_VLANID_CNS;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &(prvTgfPacketL2Part.saMac), sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

    /* Checking if there is MAC learning */
    if (rc != GT_OK)
    {
        PRV_UTF_LOG6_MAC("Device has NOT learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                prvTgfPacketL2Part.saMac[0],prvTgfPacketL2Part.saMac[1],prvTgfPacketL2Part.saMac[2],
                prvTgfPacketL2Part.saMac[3],prvTgfPacketL2Part.saMac[4],prvTgfPacketL2Part.saMac[5]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet");
    }

    /* compare entryType */
    rc = (macEntry.key.entryType == PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "(macEntry.key.entryType != MAC_ADDR_E");

    /* compare MAC address */
    rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
                      macEntryKey.key.macVlan.macAddr.arEther,
                      sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "FDB has another MAC address");

    /* compare vlanId */
    rc = (macEntry.key.key.macVlan.vlanId == macEntryKey.key.macVlan.vlanId) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "FDB has another vlanid");

    /* compare dstInterface.type */
    rc = (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "FDB has another dstInterface.type");

    /* compare dstInterface */
    if (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) {
        rc = (macEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "FDB has another dstInterface.devPort.hwDevNum");

        rc = (macEntry.dstInterface.devPort.portNum == lookupInfoStc[lookupNum].sourceEport) ? GT_OK : GT_FAIL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "FDB has another dstInterface.devPort.portNum");
    }
    /* compare isStatic */
    rc = (macEntry.isStatic == GT_FALSE) ? GT_OK : GT_FAIL;
    if (rc == GT_OK)
    {
        PRV_UTF_LOG6_MAC("Device has learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                prvTgfPacketL2Part.saMac[0],prvTgfPacketL2Part.saMac[1],prvTgfPacketL2Part.saMac[2],
                prvTgfPacketL2Part.saMac[3],prvTgfPacketL2Part.saMac[4],prvTgfPacketL2Part.saMac[5]);

        PRV_UTF_LOG1_MAC("Device has learned ePort %d\n",macEntry.dstInterface.devPort.portNum);
    }

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
}

/**
* @internal tgfTunnelTermSourceEportAssignmentConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
static GT_VOID tgfTunnelTermSourceEportAssignmentConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       lookupNum;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MPLS */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");


    for(lookupNum = 0; lookupNum < PRV_TGF_MAX_LOOKUPS_NUM_CNS; lookupNum++)
    {
        if(0 == lookupInfoStc[lookupNum].isValid)
        {
            break;
        }

        /* AUTODOC: invalidate TTI rules */
        rc = prvTgfTtiRuleValidStatusSet(lookupInfoStc[lookupNum].ttiRuleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");
    }

    /* AUTODOC: restore TCAM segment mode for Mpls key */
    rc = prvTgfTtiTcamSegmentModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, tcamSegmentModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamSegmentModeSet");

    for (lookupNum=0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS; lookupNum++)
    {
        /* Disable Bridge Control Learning on the new src eport */
        rc = prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, (PRV_TGF_NEW_SRC_EPORT_BASE_CNS + lookupNum), GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbNaToCpuPerPortSet");

        /* AUTODOC: set port auto learn mode to disable on the new src eport */
        rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, (PRV_TGF_NEW_SRC_EPORT_BASE_CNS + lookupNum),GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    firstCall=GT_TRUE;
}

/**
* @internal tgfTunnelTermSourceEportAssignment function
* @endinternal
*
* @brief   Test of TTI Source Eport Assignment (main func)
*
* @param[in] sourceEPortAssignmentEnableArray[] - array of sourceEportAssignment enable values for each lookup
* @param[in] lookupNumSourceEportExpected - the final lookup number of the expected eport value
* @param[in] restoreConfig            - GT_TRUE:restore
*                                      GT_FALSE: do not restore
*                                       None
*/
GT_VOID tgfTunnelTermSourceEportAssignment
(
    GT_BOOL sourceEPortAssignmentEnableArray[],
    GT_U32  lookupNumSourceEportExpected,
    GT_BOOL restoreConfig
)
{
    GT_STATUS   rc;
    GT_U8       lookupNum;
    GT_U8       i;

    /* AUTODOC: set TTI lookups configuration */
    for(lookupNum = 0; lookupNum < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; lookupNum++)
    {
        PRV_UTF_LOG1_MAC("\n============================ Process lookup %d =======================\n", lookupNum);

        if((0 == lookupNum)&&(firstCall==GT_TRUE))
        {
            /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
            /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
            tgfTunnelTermSourceEportAssignmentBridgeConfigSet();

            /* AUTODOC: set generic TTI configuration */
            tgfTunnelTermSourceEportAssignmentTtiConfigSet();

            for(i = 0; i < PRV_TGF_NUM_OF_TTI_LOOKUPS_CNS ; i++)
            {
                sourceEportArray[i] = PRV_TGF_NEW_SRC_EPORT_BASE_CNS + i;

                /* Enable Bridge Control Learning on the new src eport */
                rc = prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, (PRV_TGF_NEW_SRC_EPORT_BASE_CNS + i), GT_TRUE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbNaToCpuPerPortSet");

                /* AUTODOC: set port auto learn mode to enable on the new src eport */
                rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, (PRV_TGF_NEW_SRC_EPORT_BASE_CNS + i), GT_FALSE);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgFdbPortAutoLearnEnableSet");
            }
            firstCall=GT_FALSE;
        }

        /* AUTODOC: init lookup data */
        lookupInfoStc[lookupNum].isValid = 1;
        lookupInfoStc[lookupNum].ttiRuleIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(lookupNum , 3);
        lookupInfoStc[lookupNum].sourceEPortAssignmentEnable = sourceEPortAssignmentEnableArray[lookupNum];
        lookupInfoStc[lookupNum].sourceEport                 = sourceEportArray[lookupNum];

        /* AUTODOC: set the TTI Rule Action for all the packets */
        /* AUTODOC: add TTI rule 1 with MPLS key on port 0 VLAN 5 with */
        /* AUTODOC: TTI action: sourceEport Assignment modification per
                    lookupNum redirect to port 18 or 36 according to lookupNum */
        tgfTunnelTermSourceEportAssignmentTtiConfigPerLookupSet(lookupNum);
    }

    /* AUTODOC: send packet and expect TTI match, and traffic on port 36 */
    tgfTunnelTermSourceEportAssignmentTrafficGenerate(GT_TRUE,lookupNumSourceEportExpected);

    if (restoreConfig==GT_TRUE)
    {
        /*--------------------------------*/
        /* AUTODOC: test configuration restore */
        tgfTunnelTermSourceEportAssignmentConfigRestore();
    }
}


