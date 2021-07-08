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
* @file prvTgfCos.c
*
* @brief CPSS DXCH Class of Service facility implementation.
*
* @version   22
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfCosGen.h>
#include <cos/prvTgfCos.h>
#include <common/tgfBridgeGen.h>
#include <utf/private/prvUtfExtras.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Qos Profiles used in test */
#define PRV_TGF_COS_TEST_PROFILE0 6
#define PRV_TGF_COS_TEST_PROFILE1 7

/* Traffic Classes used in test */
#define PRV_TGF_COS_TEST_TC0 6
#define PRV_TGF_COS_TEST_TC1 7

/* index of port in port-array for send port */
#define PRV_TGF_COS_SEND_PORT_INDEX_CNS        1

/* index of port in port-array for send port */
#define PRV_TGF_COS_RECEIVE_PORT_INDEX_CNS     2

/* Exp value used in test */
#define PRV_TGF_COS_EXP0_CNS        1

/* User Priorities used in test */
#define PRV_TGF_COS_TEST_UP0 6
#define PRV_TGF_COS_TEST_UP1 7

/* CFI used in test */
#define PRV_TGF_COS_TEST_CFI0 0
#define PRV_TGF_COS_TEST_CFI1 0

/* User Priorities profile indexes used in test */
#define PRV_TGF_COS_TEST_UP_PROFILE_INDEX_0         0
#define PRV_TGF_COS_TEST_UP_PROFILE_INDEX_1         1

/******************************* Test packet **********************************/

/******************************* common payload *******************************/

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

/******************************* MPLS packet **********************************/

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart =
{TGF_ETHERTYPE_8847_MPLS_TAG_CNS};
/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Ipv6 = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsPart =
{
    1,                   /* label */
    PRV_TGF_COS_EXP0_CNS,/* experimental use */
    1,                  /* stack --> end of stack (last bit) */
    0x40                 /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of MPLS packet */
#define PRV_TGF_MPLS_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_MPLS_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadLongDataArr)

/* MPLS packet to send */
static TGF_PACKET_STC prvTgfMplsPacketInfo =
{
    PRV_TGF_MPLS_PACKET_LEN_CNS,                 /* totalLen */
    (sizeof(prvTgfMplsPacketPartArray)
        / sizeof(prvTgfMplsPacketPartArray[0])), /* numOfParts */
    prvTgfMplsPacketPartArray                    /* partsArray */
};

/*************** Double tagged ETH_OTHER packet *******************************/

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOthEtherTypePart =
{0x3456};

/* VLAN tag0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_COS_TEST_UP0  /*pri*/,
    PRV_TGF_COS_TEST_CFI0 /*cfi*/,
    0 /*vid - UP tagged packet */
};

/* VLAN tag1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_COS_TEST_UP1  /*pri*/,
    PRV_TGF_COS_TEST_CFI1 /*cfi*/,
    0 /*vid - UP tagged packet */
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfEthOthPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOthEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of ETH_OTHER packet */
#define PRV_TGF_ETH_OTHER_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) \
    + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* MPLS packet to send */
static TGF_PACKET_STC prvTgfEthOtherPacketInfo =
{
    PRV_TGF_ETH_OTHER_PACKET_LEN_CNS,              /* totalLen */
    (sizeof(prvTgfEthOthPacketPartArray)
        / sizeof(prvTgfEthOthPacketPartArray[0])), /* numOfParts */
    prvTgfEthOthPacketPartArray                    /* partsArray */
};

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfCosTestCommonConfigure function
* @endinternal
*
* @brief   This function configures Cos all tests
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCosTestCommonConfigure
(
    GT_VOID
)
{
    GT_STATUS                      rc;
    PRV_TGF_COS_PROFILE_STC        profileEntry = {0,0,0,0,0};
    CPSS_QOS_ENTRY_STC             cosEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    prvTgfEthCountersReset(prvTgfDevsArray[0]);/* force link up on all needed ports */
    /* Confgure Qos Profile0 */
    profileEntry.trafficClass = PRV_TGF_COS_TEST_TC0;

    /* AUTODOC: config cos profile entry 6 with: */
    /* AUTODOC:   DP=GREEN, UP=DSCP=EXP=0, TC=6 */
    rc = prvTgfCosProfileEntrySet(
        PRV_TGF_COS_TEST_PROFILE0, &profileEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosProfileEntrySet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* Confgure Qos Profile1 */
    profileEntry.trafficClass = PRV_TGF_COS_TEST_TC1;

    /* AUTODOC: config cos profile entry 7 with: */
    /* AUTODOC:   DP=GREEN, UP=DSCP=EXP=0, TC=7 */
    rc = prvTgfCosProfileEntrySet(
        PRV_TGF_COS_TEST_PROFILE1, &profileEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosProfileEntrySet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: map EXP=1 to COS profile 7 */
    rc = prvTgfCosExpToProfileMapSet( 0/*mappingTableIndex*/,
        PRV_TGF_COS_EXP0_CNS, PRV_TGF_COS_TEST_PROFILE1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosExpToProfileMapSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: map UP=6, upProfileIdx=0, CFI=0 to COS profile 6 */
    rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_TEST_UP_PROFILE_INDEX_0,
        PRV_TGF_COS_TEST_UP0, PRV_TGF_COS_TEST_CFI0,
        PRV_TGF_COS_TEST_PROFILE0);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosUpCfiDeiToProfileMapSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: map UP=7, upProfileIdx=1, CFI=0 to COS profile 7 */
    rc = prvTgfCosUpCfiDeiToProfileMapSet(0/*mappingTableIndex*/,PRV_TGF_COS_TEST_UP_PROFILE_INDEX_1,
        PRV_TGF_COS_TEST_UP1, PRV_TGF_COS_TEST_CFI1,
        PRV_TGF_COS_TEST_PROFILE1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosUpCfiDeiToProfileMapSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* map send port to Qos Profile0 */
    cosEntry.qosProfileId     = PRV_TGF_COS_TEST_PROFILE0;
    cosEntry.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    cosEntry.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    cosEntry.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* AUTODOC: config QoS entry 6 for port 1 with: */
    /* AUTODOC:   assignPrecedenc=HARD */
    /* AUTODOC:   enableModifyUp=enableModifyDscp=DISABLE */
    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS], &cosEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosConfigSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: set upProfileIdx=0 for UP=6, port=1 */
    rc = prvTgfCosPortUpProfileIndexSet(
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        PRV_TGF_COS_TEST_UP0,
        PRV_TGF_COS_TEST_UP_PROFILE_INDEX_0);
    if(rc == GT_NOT_APPLICABLE_DEVICE)
    {
        /* this device not support multiple tables */
    }
    else if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosConfigSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: set upProfileIdx=1 for UP=7, port=1 */
    rc = prvTgfCosPortUpProfileIndexSet(
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        PRV_TGF_COS_TEST_UP1,
        PRV_TGF_COS_TEST_UP_PROFILE_INDEX_1);
    if(rc == GT_NOT_APPLICABLE_DEVICE)
    {
        /* this device not support multiple tables */
    }
    else if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosConfigSet FAILED, rc = [%d]", rc);
        return rc;
    }


    return GT_OK;
}

/**
* @internal prvTgfCosTestCommonReset function
* @endinternal
*
* @brief   This function resets configuration of Cos
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCosTestCommonReset
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_QOS_ENTRY_STC             cosEntry;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* map send port to Qos Profile0 */
    cosEntry.qosProfileId     = 0;
    cosEntry.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    cosEntry.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    cosEntry.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* AUTODOC: restore default QoS entry for port 1 */
    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS], &cosEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosConfigSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: reset Trust mode for port 1 */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS], CPSS_QOS_PORT_NO_TRUST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosTrustModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: disable trust EXP mode for MPLS packets for port 1 */
    rc = prvTgfCosTrustExpModeSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS], GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosTrustModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: reset upProfileIdx=0 for UP=6, port=1 */
    rc = prvTgfCosPortUpProfileIndexSet(
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        PRV_TGF_COS_TEST_UP0,
        0);
    if(rc == GT_NOT_APPLICABLE_DEVICE)
    {
        /* this device not support multiple tables */
    }
    else if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosConfigSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: reset upProfileIdx=0 for UP=7, port=1 */
    rc = prvTgfCosPortUpProfileIndexSet(
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        PRV_TGF_COS_TEST_UP1,
        0);
    if(rc == GT_NOT_APPLICABLE_DEVICE)
    {
        /* this device not support multiple tables */
    }
    else if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosConfigSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfBrgFdbFlush FAILED: %d, %d", prvTgfDevNum, GT_TRUE);
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvTgfCosTestSendPacketAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets of given Traffic classes
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               -  burst size
* @param[in] tc0                      -  traffic class to count in counter set0
* @param[in] tc1                      -  traffic class to count in counter set1
* @param[in] expectedEgressTc0        - amount of packets expected to egress With TC0
* @param[in] expectedEgressTc1        - amount of packets expected to egress With TC1
*                                       None
*/
static GT_VOID prvTgfCosTestSendPacketAndCheck
(
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32            burstCount,
    IN GT_U8             tc0,
    IN GT_U8             tc1,
    IN GT_U32            expectedEgressTc0,
    IN GT_U32            expectedEgressTc1
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U8                           cntrSetId;
    CPSS_PORT_EGRESS_CNTR_STC       egressCntrs;
    GT_U32                          egressPktSum;
    GT_U8                           tcArr[2];
    GT_U32                          expectedPackets[2];

    tcArr[0] = tc0;
    tcArr[1] = tc1;

    expectedPackets[0] = expectedEgressTc0;
    expectedPackets[1] = expectedEgressTc1;

    PRV_UTF_LOG0_MAC("======= Configure Counters Sets =======\n");

    for (cntrSetId = 0; (cntrSetId < 2); cntrSetId++)
    {
        rc = prvTgfPortEgressCntrModeSet(
            cntrSetId,
            (CPSS_EGRESS_CNT_TC_E | CPSS_EGRESS_CNT_PORT_E) /*setModeBmp*/,
            prvTgfPortsArray[PRV_TGF_COS_RECEIVE_PORT_INDEX_CNS] /*portNum*/,
            0 /*vlanId*/,
            tcArr[cntrSetId]/*tc*/,
            CPSS_DP_GREEN_E /*dpLevel*/);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPortEgressCntrModeSet #%d, port:%d, tc:%d\n",
                            cntrSetId, prvTgfPortsArray[PRV_TGF_COS_RECEIVE_PORT_INDEX_CNS], tcArr[cntrSetId]);
    }

    PRV_UTF_LOG0_MAC("======= Reset Counters =======\n");

    for (cntrSetId = 0; (cntrSetId < 2); cntrSetId++)
    {
        /* reset counters */
        rc = prvTgfPortEgressCntrsGet(
            cntrSetId, &egressCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPortEgressCntrsGet #%d\n", cntrSetId);
    }

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        packetInfoPtr, burstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS], burstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth: %d %d",
        prvTgfDevsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS]);

    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);


    for (cntrSetId = 0; (cntrSetId < 2); cntrSetId++)
    {
        /* read counters */
        rc = prvTgfPortEgressCntrsGet(
            cntrSetId, &egressCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPortEgressCntrsGet\n");

        egressPktSum = egressCntrs.outUcFrames + egressCntrs.outMcFrames
            + egressCntrs.outCtrlFrames + egressCntrs.outBcFrames;

        UTF_VERIFY_EQUAL1_STRING_MAC(
            expectedPackets[cntrSetId], egressPktSum, "Egress Counter #%d result\n", cntrSetId);
    }

}


/**
* @internal prvTgfCosExpTestTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets with both TCs
*/
GT_VOID prvTgfCosExpTestTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                 rc;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: enable trust EXP mode for MPLS packets for port 1 */
    rc = prvTgfCosTrustExpModeSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS], GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosTrustModeSet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: send 2 MPLS packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   EtherType=0x8847, label=1, exp=1, stack=0, ttl=0x40 */
    prvTgfCosTestSendPacketAndCheck(
        &prvTgfMplsPacketInfo,
        2 /*burstCount*/,
        PRV_TGF_COS_TEST_TC0 /*tc0*/,
        PRV_TGF_COS_TEST_TC1 /*tc1*/,
        0 /*expectedEgressTc0*/,
        2 /*expectedEgressTc1*/);
    /* AUTODOC: verify egress counters: */
    /* AUTODOC:   0 packets with TC=6 */
    /* AUTODOC:   2 packets with TC=7 */

    /* AUTODOC: disable trust EXP mode for MPLS packets for port 1 */
    rc = prvTgfCosTrustExpModeSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS], GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosTrustModeSet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: send MPLS packet from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   EtherType=0x8847, label=1, exp=1, stack=0, ttl=0x40 */
    prvTgfCosTestSendPacketAndCheck(
        &prvTgfMplsPacketInfo,
        1 /*burstCount*/,
        PRV_TGF_COS_TEST_TC0 /*tc0*/,
        PRV_TGF_COS_TEST_TC1 /*tc1*/,
        1 /*expectedEgressTc0*/,
        0 /*expectedEgressTc1*/);
    /* AUTODOC: verify egress counters: */
    /* AUTODOC:   1 packets with TC=6 */
    /* AUTODOC:   0 packets with TC=7 */
}

/**
* @internal prvTgfCosVlanTagSelectTestTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets with both TCs
*/
GT_VOID prvTgfCosVlanTagSelectTestTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS                 rc;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: set Qos Trust Mode as TRUST_L2 for port 1 */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        CPSS_QOS_PORT_TRUST_L2_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosPortQosTrustModeSet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: select VLAN_TAG1 type on port 1 for Trust L2 Qos mode */
    rc = prvTgfCosL2TrustModeVlanTagSelectSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        GT_TRUE,
        CPSS_VLAN_TAG1_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosL2TrustModeVlanTagSelectSet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: send 2 Ethernet packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   TAG0: UP=6, CFI=0, VID=0 */
    /* AUTODOC:   TAG1: UP=7, CFI=0, VID=0 */
    /* AUTODOC:   EtherType=0x3456 */
    prvTgfCosTestSendPacketAndCheck(
        &prvTgfEthOtherPacketInfo,
        2 /*burstCount*/,
        PRV_TGF_COS_TEST_TC0 /*tc0*/,
        PRV_TGF_COS_TEST_TC1 /*tc1*/,
        0 /*expectedEgressTc0*/,
        2 /*expectedEgressTc1*/);
    /* AUTODOC: verify egress counters: */
    /* AUTODOC:   0 packets with TC=6 */
    /* AUTODOC:   2 packets with TC=7 */

    /* AUTODOC: select VLAN_TAG0 type on port 1 for Trust L2 Qos mode */
    rc = prvTgfCosL2TrustModeVlanTagSelectSet(
        prvTgfPortsArray[PRV_TGF_COS_SEND_PORT_INDEX_CNS],
        GT_TRUE,
        CPSS_VLAN_TAG0_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: prvTgfCosL2TrustModeVlanTagSelectSet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: send Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:00, SA=00:66:77:88:99:00 */
    /* AUTODOC:   TAG0: UP=6, CFI=0, VID=0 */
    /* AUTODOC:   TAG1: UP=7, CFI=0, VID=0 */
    /* AUTODOC:   EtherType=0x3456 */
    prvTgfCosTestSendPacketAndCheck(
        &prvTgfEthOtherPacketInfo,
        1 /*burstCount*/,
        PRV_TGF_COS_TEST_TC0 /*tc0*/,
        PRV_TGF_COS_TEST_TC1 /*tc1*/,
        1 /*expectedEgressTc0*/,
        0 /*expectedEgressTc1*/);
    /* AUTODOC: verify egress counters: */
    /* AUTODOC:   1 packets with TC=6 */
    /* AUTODOC:   0 packets with TC=7 */
}

typedef struct{
    GT_U8  dscp;
    GT_U8  newDscp;
}COS_DSCP_TO_DSCP_STC;

typedef struct{
    GT_U8  x; /*used as dscp / up / exp */
    GT_U8  y;/* relevant only to upCfiToQosProfile - used as CFI */
    GT_U32  qosProfile;
}COS_X_Y_TO_PROFILE_STC;

typedef struct{
    GT_U32                  mappingTableIndex;

    COS_DSCP_TO_DSCP_STC    dscpMutation;
    COS_X_Y_TO_PROFILE_STC  dscpToQosProfile;
    COS_X_Y_TO_PROFILE_STC  upCfiToQosProfile;
    COS_X_Y_TO_PROFILE_STC  expToQosProfile;

}COS_MAPPING_TABLE_INDEX_INFO_STC;

#define MAPPING_TABLE_SIZE  3

/* array of qos profile index that derived from trust DSCP */
static GT_U32   profileFromDscpArr[MAPPING_TABLE_SIZE]  = {100,106,80};
/* array of qos profile index that derived from trust UP */
static GT_U32   profileFromUpCfiArr[MAPPING_TABLE_SIZE] = {101,116,110};
/* array of qos profile index that derived from trust EXP */
static GT_U32   profileFromExpArr[MAPPING_TABLE_SIZE]   = {10,11,12};
/* array of new DSCP (mutation) that derived from old DSCP */
static GT_U8   newDscpArr[MAPPING_TABLE_SIZE]          = {40,20,25};

/* DSCP to use on the tested packet */
static GT_U8   packetDscp  = 30;
/* UP to use on the tested packet */
static GT_U8   packetUp    = 5;
/* CFI to use on the tested packet */
static GT_U8   packetCfi   = 0;
/* EXP to use on the tested packet */
static GT_U8   packetExp   = 4;
/* vid to use on the tested packet */
static GT_U16   packetVid  = 0x89A;

/* index in mappingTableIndexArr[] to be used as 'UP' and not from the mappingTableIndexArr[] */
#define TESTED_INDEX_BY_UP      1

/* mapping table index to use in the tests */
static GT_U32   mappingTableIndexArr[MAPPING_TABLE_SIZE]  = {7 , 5 /* from UP */, 11};

#define FILLED_IN_RUNTIME   0

static COS_MAPPING_TABLE_INDEX_INFO_STC  cosMappingTableInfoArr[MAPPING_TABLE_SIZE]=
{
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME}     /* filled in runtime */
};

static COS_MAPPING_TABLE_INDEX_INFO_STC  origCosMappingTableInfoArr[MAPPING_TABLE_SIZE]=
{
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME}     /* filled in runtime */
};

typedef struct{
    GT_U32                  qosProfile;
    PRV_TGF_COS_PROFILE_STC entry;
}QOS_ENTRY_STC;

/* UP on the egress packet that selected according to the DSCP to qos Index */
static GT_U32   egressUpFromDscpArr[MAPPING_TABLE_SIZE]  = {6,1,3};
/* UP on the egress packet that selected according to the UP,cfi to qos Index */
static GT_U32   egressUpFromUpCfiArr[MAPPING_TABLE_SIZE] = {0,2,4};
/* UP on the egress packet that selected according to the EXP to qos Index */
static GT_U32   egressUpFromExpArr[MAPPING_TABLE_SIZE]   = {2,3,6};

/* DSCP on the egress packet that selected according to the DSCP to qos Index */
static GT_U32   egressDscpFromDscpArr[MAPPING_TABLE_SIZE]  = {31,41,45};
/* DSCP on the egress packet that selected according to the UP,cfi to qos Index */
static GT_U32   egressDscpFromUpCfiArr[MAPPING_TABLE_SIZE] = {12,42,55};
/* DSCP on the egress packet that selected according to the EXP to qos Index */
static GT_U32   egressDscpFromExpArr[MAPPING_TABLE_SIZE]   = {0,0,0};

/* EXP on the egress packet that selected according to the DSCP to qos Index */
static GT_U32   egressExpFromDscpArr[MAPPING_TABLE_SIZE]  = {0,0,0};
/* EXP on the egress packet that selected according to the UP,cfi to qos Index */
static GT_U32   egressExpFromUpCfiArr[MAPPING_TABLE_SIZE] = {0,7,0};
/* EXP on the egress packet that selected according to the EXP to qos Index */
static GT_U32   egressExpFromExpArr[MAPPING_TABLE_SIZE]   = {5,6,1};
/* 3 cases tested in each mapping table index : use of trust L2 , trust L3 , use UP from packet */
#define NUM_OF_TEST_CASES   3

static QOS_ENTRY_STC cosQosTableInfoArr[3*MAPPING_TABLE_SIZE]={
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME}     /* filled in runtime */
};

static QOS_ENTRY_STC origCosQosTableInfoArr[3*MAPPING_TABLE_SIZE]={
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME}     /* filled in runtime */
};

/*
for: mappingTableIndex = 7 (from port 8 with index)
set dscp-to-dscp                 :from 30 to new dscp 40
set dscp-to-profile                 :from 40 to qosIndex 100
set qosProfileTable entry 100 : DSCP = 31 , UP = 6
set up,cfi-to-profile                 : from up=5 and cfi=0 to qosIndex 101
set qosProfileTable entry 101 : DSCP = 12 , UP = 0
set EXP-to-Profile                : from 4 to qosIndex 10
set qosProfileTable entry 10  : EXP = 2 , UP = 2

for: mappingTableIndex = 5 (from port 8 with UP)
set dscp-to-dscp                 :from 30 to new dscp 20
set dscp-to-profile                 :from 20 to qosIndex 106
set qosProfileTable entry 106 : DSCP = 41 , UP = 1
set up,cfi-to-profile                 : from up=5 and cfi=0 to qosIndex 116
set qosProfileTable entry 116 : DSCP = 42 , UP = 2 , EXP = 7
set EXP-to-Profile                : from 4 to qosIndex 11
set qosProfileTable entry 11  : EXP = 3 , UP = 3

for: mappingTableIndex = 11(from port 8 with index)
set dscp-to-dscp                 : from 30 to new dscp 25
set dscp-to-profile                 : from 25 to qosIndex 80
set qosProfileTable entry 80: DSCP = 45 , UP = 3
set up,cfi-to-profile                 : from up=5 and cfi=0 to qosIndex 110
set qosProfileTable entry 110: DSCP = 55 , UP = 4
set EXP-to-Profile                : from 4 to qosIndex 12
set qosProfileTable entry 12  : EXP = 6 , UP = 6

on default ePort 8 ' 'modifyUp = GT_TRUE' and 'modifyDscp/EXP = GT_TRUE'
on default ePort 18 ' 'modifyUp = GT_TRUE' and 'modifyDscp/EXP = GT_TRUE'

Mac DA of all packets in those tests is associated with port 23. (all traffic ingress port 8/18 and egress port 23)

*/

/* VLAN tag0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag2Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    0  /*pri - set at runtime */,
    0 /*cfi* - set at runtime */,
    FILLED_IN_RUNTIME /* vid - filled in runtime */
};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacket1MplsPart =
{
    1,                   /* label */
    FILLED_IN_RUNTIME,   /* EXP - filled in runtime */
    1,                   /* stack --> end of stack (last bit) */
    0x40                 /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfMplsPacket1PartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacket1MplsPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* MPLS packet to send */
static TGF_PACKET_STC prvTgfMplsPacket1Info =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                 /* totalLen */
    (sizeof(prvTgfMplsPacket1PartArray)
        / sizeof(prvTgfMplsPacket1PartArray[0])), /* numOfParts */
    prvTgfMplsPacket1PartArray                    /* partsArray */
};


/* packet's IPv4 */
/* SIP[5:0]==1, DIP[5:1]==0 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OtherPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    FILLED_IN_RUNTIME,  /* typeOfService - filled in runtime */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 7,  0,  8,   0},    /* srcAddr */
    { 1,  1,  1,   0}    /* dstAddr */
};

/* PARTS of packet IPv4 - IPV4 OTHER */
static TGF_PACKET_PART_STC prvTgfIpv4OtherPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* IPv4 Other packet to send */
static TGF_PACKET_STC prvTgfIpv4OtherPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                      /* totalLen */
    (sizeof(prvTgfIpv4OtherPacketPartArray)
        / sizeof(prvTgfIpv4OtherPacketPartArray[0])), /* numOfParts */
    prvTgfIpv4OtherPacketPartArray                    /* partsArray */
};

#define IPV6_DUMMY_PROTOCOL_CNS  0x3b

/* IPv6 header */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    FILLED_IN_RUNTIME,  /* trafficClass - filled in runtime */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    IPV6_DUMMY_PROTOCOL_CNS, /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0xff02, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* IPv6 PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* IPv6 PACKET to send */
static TGF_PACKET_STC prvTgfPacketIpv6Info = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                  /* totalLen */
    sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]), /* numOfParts */
    prvTgfIpv6PacketArray                                        /* partsArray */
};

typedef enum{
    MAPPING_TABLE_INDEX_PACKET_TYPE_IPV4_E,
    MAPPING_TABLE_INDEX_PACKET_TYPE_IPV6_E,
    MAPPING_TABLE_INDEX_PACKET_TYPE_MPLS_E,

    MAPPING_TABLE_INDEX_PACKET_TYPE____LAST____E
}MAPPING_TABLE_INDEX_PACKET_TYPE_ENT;

static TGF_PACKET_STC* mappingTableIndexPacketsArr[MAPPING_TABLE_INDEX_PACKET_TYPE____LAST____E] =
{
    &prvTgfIpv4OtherPacketInfo,
    &prvTgfPacketIpv6Info,
    &prvTgfMplsPacket1Info
};


#define NUM_TRUST_MODE  2
static CPSS_QOS_PORT_TRUST_MODE_ENT mappingTableIndexTrustModeArr[NUM_TRUST_MODE] =
{
    CPSS_QOS_PORT_TRUST_L3_E,
    CPSS_QOS_PORT_TRUST_L2_E
};

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 1;

#define DUMMY_EXPECTED_PACKET_PART              \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL},       \
    {TGF_PACKET_PART_SKIP_E,       NULL}

/* dummy expected packet */
static TGF_PACKET_PART_STC dummyExpectedPacketPartArray[] =
{
    DUMMY_EXPECTED_PACKET_PART
};

/* original dummy expected packet */
static TGF_PACKET_PART_STC origDummyExpectedPacketPartArray[] =
{
    DUMMY_EXPECTED_PACKET_PART
};

#define L2_PART_INDEX       1
#define L3_PART_INDEX       3


/* dummy expected packet */
static TGF_PACKET_STC dummyExpectedPacket =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,      /* totalLen */
    sizeof(dummyExpectedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    dummyExpectedPacketPartArray                                        /* partsArray */
};

static GT_BOOL  origUseUpAsIndex = GT_FALSE;
static GT_U32   origMappingTableIndex = 0;
static CPSS_QOS_PORT_TRUST_MODE_ENT origTrustMode = CPSS_QOS_PORT_NO_TRUST_E;
static GT_BOOL  origTrunsExp = GT_FALSE;
static CPSS_QOS_ENTRY_STC  origPortQosParam;
static GT_BOOL  origEnableDscpMutation = GT_FALSE;


/* configure the cosMappingTableInfoArr,cosQosTableInfoArr and the device with the 3 instances of tables */
static void mappingTableIndexConfig(
    IN GT_U32            testedPortIndex,
    IN GT_BOOL           startOrRestore
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    CPSS_QOS_ENTRY_STC  portQosParam;

    /*replacing the FILLED_IN_RUNTIME*/

    if(startOrRestore == GT_TRUE)/* start */
    {
        /* init cosMappingTableInfoArr[] ... allow pre-test configuration to change the 'default values' */
        for(ii = 0 ; ii < MAPPING_TABLE_SIZE; ii++)
        {
            cosMappingTableInfoArr[ii].mappingTableIndex        = mappingTableIndexArr[ii];
            cosMappingTableInfoArr[ii].dscpMutation.dscp        = packetDscp;
            cosMappingTableInfoArr[ii].dscpMutation.newDscp     = newDscpArr[ii];
            cosMappingTableInfoArr[ii].dscpToQosProfile.x       = cosMappingTableInfoArr[ii].dscpMutation.newDscp;
            cosMappingTableInfoArr[ii].dscpToQosProfile.qosProfile = profileFromDscpArr[ii];
            cosMappingTableInfoArr[ii].upCfiToQosProfile.x      = packetUp;
            cosMappingTableInfoArr[ii].upCfiToQosProfile.y      = packetCfi;
            cosMappingTableInfoArr[ii].upCfiToQosProfile.qosProfile = profileFromUpCfiArr[ii];
            cosMappingTableInfoArr[ii].expToQosProfile.x        = packetExp;
            cosMappingTableInfoArr[ii].expToQosProfile.qosProfile = profileFromExpArr[ii];

            rc = prvTgfCosDscpMutationEntryGet(prvTgfDevNum,
                cosMappingTableInfoArr[ii].mappingTableIndex,
                cosMappingTableInfoArr[ii].dscpMutation.dscp,
                &origCosMappingTableInfoArr[ii].dscpMutation.newDscp);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosDscpMutationEntryGet");


            rc = prvTgfCosDscpMutationEntrySet(prvTgfDevNum,
                cosMappingTableInfoArr[ii].mappingTableIndex,
                cosMappingTableInfoArr[ii].dscpMutation.dscp,
                cosMappingTableInfoArr[ii].dscpMutation.newDscp);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosDscpMutationEntrySet");

            rc = prvTgfCosExpToProfileMapGet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                             cosMappingTableInfoArr[ii].expToQosProfile.x,
                                             &origCosMappingTableInfoArr[ii].expToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosExpToProfileMapGet");

            rc = prvTgfCosExpToProfileMapSet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                             cosMappingTableInfoArr[ii].expToQosProfile.x,
                                             cosMappingTableInfoArr[ii].expToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosExpToProfileMapSet");

            rc = prvTgfCosDscpToProfileMapGet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                             cosMappingTableInfoArr[ii].dscpToQosProfile.x,
                                             &origCosMappingTableInfoArr[ii].dscpToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosDscpToProfileMapGet");

            rc = prvTgfCosDscpToProfileMapSet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                             cosMappingTableInfoArr[ii].dscpToQosProfile.x,
                                             cosMappingTableInfoArr[ii].dscpToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosDscpToProfileMapSet");

            rc = prvTgfCosUpCfiDeiToProfileMapGet(prvTgfDevNum,
                                                cosMappingTableInfoArr[ii].mappingTableIndex,
                                                0,/*upProfileIndex --> not relevant to those devices */
                                                cosMappingTableInfoArr[ii].upCfiToQosProfile.x,
                                                cosMappingTableInfoArr[ii].upCfiToQosProfile.y,
                                                &origCosMappingTableInfoArr[ii].upCfiToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosUpCfiDeiToProfileMapGet");

            rc = prvTgfCosUpCfiDeiToProfileMapSet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                                0,/*upProfileIndex --> not relevant to those devices */
                                                cosMappingTableInfoArr[ii].upCfiToQosProfile.x,
                                                cosMappingTableInfoArr[ii].upCfiToQosProfile.y,
                                                cosMappingTableInfoArr[ii].upCfiToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosUpCfiDeiToProfileMapSet");
        }

        /* init cosQosTableInfoArr[] ... allow pre-test configuration to change the 'default values' */
        for(ii = 0 ; ii < (3*MAPPING_TABLE_SIZE); ii++)
        {
            cosQosTableInfoArr[ii].qosProfile           = profileFromDscpArr[ii / 3];
            cosQosTableInfoArr[ii].entry.userPriority   = egressUpFromDscpArr[ii / 3];
            cosQosTableInfoArr[ii].entry.dscp           = egressDscpFromDscpArr[ii / 3];
            cosQosTableInfoArr[ii].entry.exp            = egressExpFromDscpArr[ii / 3];

            rc = prvTgfCosProfileEntryGet(prvTgfDevNum,cosQosTableInfoArr[ii].qosProfile,
                                &origCosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntryGet");

            rc = prvTgfCosProfileEntrySet(cosQosTableInfoArr[ii].qosProfile,&cosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");

            ii++;
            cosQosTableInfoArr[ii].qosProfile           = profileFromUpCfiArr[ii / 3];
            cosQosTableInfoArr[ii].entry.userPriority   = egressUpFromUpCfiArr[ii / 3];
            cosQosTableInfoArr[ii].entry.dscp           = egressDscpFromUpCfiArr[ii / 3];
            cosQosTableInfoArr[ii].entry.exp            = egressExpFromUpCfiArr[ii / 3];

            rc = prvTgfCosProfileEntryGet(prvTgfDevNum,cosQosTableInfoArr[ii].qosProfile,
                                &origCosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntryGet");

            rc = prvTgfCosProfileEntrySet(cosQosTableInfoArr[ii].qosProfile,&cosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");


            ii++;
            cosQosTableInfoArr[ii].qosProfile           = profileFromExpArr[ii / 3];
            cosQosTableInfoArr[ii].entry.userPriority   = egressUpFromExpArr[ii / 3];
            cosQosTableInfoArr[ii].entry.dscp           = egressDscpFromExpArr[ii / 3];
            cosQosTableInfoArr[ii].entry.exp            = egressExpFromExpArr[ii / 3];

            rc = prvTgfCosProfileEntryGet(prvTgfDevNum,cosQosTableInfoArr[ii].qosProfile,
                                &origCosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntryGet");

            rc = prvTgfCosProfileEntrySet(cosQosTableInfoArr[ii].qosProfile,&cosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");

        }

        /* set COS parameters on the packets */
        prvTgfPacketVlanTag2Part.vid = packetVid;
        prvTgfPacketVlanTag2Part.pri = packetUp;
        prvTgfPacketVlanTag2Part.cfi = packetCfi;
        /* set ipv4 typeOfService according to dscp */
        prvTgfPacketIpv4OtherPart.typeOfService     = TGF_PACKET_IPV4_DSCP_TO_TYPE_OF_SERVICE_MAC(packetDscp);
        /* set ipv6 trafficClass according to dscp */
        prvTgfPacketIpv6Part.trafficClass           = TGF_PACKET_IPV6_DSCP_TO_TRAFFIC_CLASS_MAC(packetDscp);
        prvTgfPacket1MplsPart.exp = packetExp;

        rc = prvTgfCosPortQosTrustModeGet(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],
            &origTrustMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosPortQosTrustModeGet");

        rc = prvTgfCosTrustExpModeGet(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],
            &origTrunsExp);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosTrustExpModeGet");

        rc = prvTgfCosPortTrustQosMappingTableIndexGet(
            prvTgfDevsArray[testedPortIndex],
            prvTgfPortsArray[testedPortIndex],
            &origUseUpAsIndex,
            &origMappingTableIndex
            );
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosPortTrustQosMappingTableIndexGet");

        rc = prvTgfCosPortQosConfigGet(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],&origPortQosParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigGet");

        portQosParam.qosProfileId = 0;/* will not be used */
        portQosParam.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        portQosParam.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
        portQosParam.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

        rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[testedPortIndex],&portQosParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

        rc = prvTgfCosPortReMapDSCPGet(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],&origEnableDscpMutation);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortReMapDSCPSet");

        rc = prvTgfCosPortReMapDSCPSet(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortReMapDSCPSet");

    }
    else    /* restore */
    {
        for(ii = 0 ; ii < MAPPING_TABLE_SIZE; ii++)
        {
            rc = prvTgfCosDscpMutationEntrySet(prvTgfDevNum,
                cosMappingTableInfoArr[ii].mappingTableIndex,
                cosMappingTableInfoArr[ii].dscpMutation.dscp,
                origCosMappingTableInfoArr[ii].dscpMutation.newDscp);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosDscpMutationEntrySet");

            rc = prvTgfCosExpToProfileMapSet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                             cosMappingTableInfoArr[ii].expToQosProfile.x,
                                             origCosMappingTableInfoArr[ii].expToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosExpToProfileMapSet");

            rc = prvTgfCosDscpToProfileMapSet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                             cosMappingTableInfoArr[ii].dscpToQosProfile.x,
                                             origCosMappingTableInfoArr[ii].dscpToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosDscpToProfileMapSet");

            rc = prvTgfCosUpCfiDeiToProfileMapSet(cosMappingTableInfoArr[ii].mappingTableIndex,
                                                0,/*upProfileIndex --> not relevant to those devices */
                                                cosMappingTableInfoArr[ii].upCfiToQosProfile.x,
                                                cosMappingTableInfoArr[ii].upCfiToQosProfile.y,
                                                origCosMappingTableInfoArr[ii].upCfiToQosProfile.qosProfile);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosUpCfiDeiToProfileMapSet");
        }

        for(ii = 0 ; ii < (3*MAPPING_TABLE_SIZE); ii++)
        {
            rc = prvTgfCosProfileEntrySet(cosQosTableInfoArr[ii].qosProfile,&origCosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");

            ii++;
            rc = prvTgfCosProfileEntrySet(cosQosTableInfoArr[ii].qosProfile,&origCosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");


            ii++;
            rc = prvTgfCosProfileEntrySet(cosQosTableInfoArr[ii].qosProfile,&origCosQosTableInfoArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");

        }

        rc = prvTgfCosPortTrustQosMappingTableIndexSet(
            prvTgfDevsArray[testedPortIndex],
            prvTgfPortsArray[testedPortIndex],
            origUseUpAsIndex,
            origMappingTableIndex
            );
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosPortTrustQosMappingTableIndexSet");

        rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[testedPortIndex],&origPortQosParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

        rc = prvTgfCosPortReMapDSCPSet(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],origEnableDscpMutation);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortReMapDSCPSet");

    }
}

/**
* @internal mappingTableIndexSet function
* @endinternal
*
* @brief   set the tested port with the mappingTableIndex .
*
* @param[in] mappingTableIndex        - mapping Table Index
* @param[in] testedPortIndex          - the port to send traffic from
*                                       None
*/
static void mappingTableIndexSet(
    IN GT_U32      mappingTableIndex,
    IN GT_U32      testedPortIndex
)
{
    GT_STATUS  rc;

    rc = prvTgfCosPortTrustQosMappingTableIndexSet(
        prvTgfDevsArray[testedPortIndex],
        prvTgfPortsArray[testedPortIndex],
        (mappingTableIndex == TESTED_INDEX_BY_UP) ? GT_TRUE /*useUpAsIndex*/: GT_FALSE,
        (mappingTableIndex == TESTED_INDEX_BY_UP) ? 0x5/*must use valid value*/: mappingTableIndexArr[mappingTableIndex]
        );
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortTrustQosMappingTableIndexSet");
}

/* force link up on all tested ports */
static void forceLinkUpOnAllTestedPorts
(
    void
)
{
    GT_STATUS    rc;
    GT_U32  portIter;/* port iterator */
    CPSS_INTERFACE_INFO_STC portInterface;

    PRV_UTF_LOG0_MAC("======= force links up =======\n");

    portInterface.type            = CPSS_INTERFACE_PORT_E;

    /* force linkup on all ports involved */
    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        portInterface.devPort.hwDevNum  = prvTgfDevsArray[portIter];
        portInterface.devPort.portNum = prvTgfPortsArray[portIter];

        if(portInterface.devPort.hwDevNum != prvTgfDevNum)
        {
            continue;
        }

        rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface,GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
}

/**
* @internal captureEgressTrafficOnPort function
* @endinternal
*
* @brief   start/stop capturing on specific port of the test .
*/
static void captureEgressTrafficOnPort
(
    IN GT_BOOL      start,
    IN GT_U32       capturedPortIndex
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevsArray[capturedPortIndex];
    portInterface.devPort.portNum = prvTgfPortsArray[capturedPortIndex];

    if(start ==GT_FALSE)
    {
        /* wait for packets come to CPU */
        (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount , 500, NULL);
    }

    /* Start/Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, start);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, start);
}

/**
* @internal checkCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test .
*
* @param[in] packetType               - packet type (ipv4,ipv6,MPLS)
* @param[in] capturedPortIndex        - the captured port index to check
* @param[in] sentPacketInfoPtr        - the packet that was sent (as reference to most parts of the expected packet)
* @param[in] expectedUp               - the expected UP
* @param[in] expectedDscp             - the expected DSCP    (relevant to : ipv4,ipv6)
* @param[in] expectedExp              - the expected EXP      (relevant to : mpls)
*                                       None
*/
static void checkCaptureEgressTrafficOnPort
(
    IN MAPPING_TABLE_INDEX_PACKET_TYPE_ENT  packetType,
    IN GT_U32       capturedPortIndex,
    IN TGF_PACKET_STC  *sentPacketInfoPtr,
    IN GT_U8        expectedUp,
    IN GT_U8        expectedDscp,
    IN GT_U8        expectedExp
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC *packetInfoPtr;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    static TGF_PACKET_VLAN_TAG_STC expectedVlanTagPart;
    static TGF_PACKET_MPLS_STC     expectedMplsPart;
    static TGF_PACKET_IPV4_STC     expectedIpv4Part;
    static TGF_PACKET_IPV6_STC     expectedIpv6Part;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[capturedPortIndex];

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* copy the content from the 'Original dummy' before we modify the dummy */
    cpssOsMemCpy(dummyExpectedPacketPartArray,
        origDummyExpectedPacketPartArray,
        sizeof(dummyExpectedPacketPartArray));

    /* modify the dummy according to the sender */
    cpssOsMemCpy(dummyExpectedPacketPartArray,
        sentPacketInfoPtr->partsArray,
        sentPacketInfoPtr->numOfParts * sizeof(TGF_PACKET_PART_STC));

    if(sentPacketInfoPtr->partsArray[L2_PART_INDEX].type != TGF_PACKET_PART_VLAN_TAG_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,
                                     "dummyExpectedPacketPartArray[L2_PART_INDEX].type != TGF_PACKET_PART_VLAN_TAG_E");
        return;
    }

    cpssOsMemCpy(&expectedVlanTagPart,
                sentPacketInfoPtr->partsArray[L2_PART_INDEX].partPtr,
                sizeof(expectedVlanTagPart));

    /* expected UP */
    expectedVlanTagPart.pri = expectedUp;
    dummyExpectedPacketPartArray[L2_PART_INDEX].partPtr = &expectedVlanTagPart;

    /* modify the dummy according to expected modifications of the test */
    switch(packetType)
    {
        case (MAPPING_TABLE_INDEX_PACKET_TYPE_IPV4_E):
            cpssOsMemCpy(&expectedIpv4Part,
                        sentPacketInfoPtr->partsArray[L3_PART_INDEX].partPtr,
                        sizeof(expectedIpv4Part));

            /* expected DSCP */
            expectedIpv4Part.typeOfService = TGF_PACKET_IPV4_DSCP_TO_TYPE_OF_SERVICE_MAC(expectedDscp);
            dummyExpectedPacketPartArray[L3_PART_INDEX].partPtr = &expectedIpv4Part;

            PRV_UTF_LOG2_MAC("ipv4 expected : up[%d] , dscp[%d] \n", expectedUp,expectedDscp);
            break;
        case (MAPPING_TABLE_INDEX_PACKET_TYPE_IPV6_E):
            cpssOsMemCpy(&expectedIpv6Part,
                        sentPacketInfoPtr->partsArray[L3_PART_INDEX].partPtr,
                        sizeof(expectedIpv6Part));

            /* expected DSCP */
            expectedIpv6Part.trafficClass = TGF_PACKET_IPV6_DSCP_TO_TRAFFIC_CLASS_MAC(expectedDscp);
            dummyExpectedPacketPartArray[L3_PART_INDEX].partPtr = &expectedIpv6Part;

            PRV_UTF_LOG2_MAC("ipv6 expected : up[%d] , dscp[%d] \n", expectedUp,expectedDscp);
            break;
        case (MAPPING_TABLE_INDEX_PACKET_TYPE_MPLS_E):
            cpssOsMemCpy(&expectedMplsPart,
                        sentPacketInfoPtr->partsArray[L3_PART_INDEX].partPtr,
                        sizeof(expectedMplsPart));

            /* expected DSCP */
            expectedMplsPart.exp = expectedExp;
            dummyExpectedPacketPartArray[L3_PART_INDEX].partPtr = &expectedMplsPart;

            PRV_UTF_LOG2_MAC("mpls expected : up[%d] , exp[%d] \n", expectedUp,expectedExp);
            break;
        default:
            break;
    }

    packetInfoPtr = &dummyExpectedPacket;

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            packetInfoPtr,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
}

/**
* @internal sendPacketAndCheck function
* @endinternal
*
* @brief   send the packet from ingress port and check that the egress packet (captured)
*         is as expected.
* @param[in] packetType               - packet type (ipv4,ipv6,MPLS)
* @param[in] testedPortIndex          - the port to send traffic from
* @param[in] capturedPortIndex        - the captured port index to check
* @param[in] trustMode                - the COS trust mode.
* @param[in] sentPacketInfoPtr        - the packet to sent (and also used as reference to most parts of the expected packet)
* @param[in] expectedUp               - the expected UP
* @param[in] expectedDscp             - the expected DSCP    (relevant to : ipv4,ipv6)
* @param[in] expectedExp              - the expected EXP      (relevant to : mpls)
*                                       None
*/
static void sendPacketAndCheck(
    IN MAPPING_TABLE_INDEX_PACKET_TYPE_ENT  packetType,
    IN GT_U32                           testedPortIndex,
    IN GT_U32                           capturedPortIndex,
    IN CPSS_QOS_PORT_TRUST_MODE_ENT     trustMode,
    IN TGF_PACKET_STC                   *sentPacketInfoPtr,
    IN GT_U8                            expectedUp,
    IN GT_U8                            expectedDscp,
    IN GT_U8                            expectedExp
)
{
    GT_STATUS   rc;
    GT_BOOL trustUp;

    trustUp = (trustMode == CPSS_QOS_PORT_TRUST_L2_E || trustMode == CPSS_QOS_PORT_TRUST_L2_L3_E);

    if(packetType == MAPPING_TABLE_INDEX_PACKET_TYPE_MPLS_E)
    {
        /* trust UP mode - not trust DSCP */
        rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[testedPortIndex],
            (trustUp == GT_TRUE) ?
            CPSS_QOS_PORT_TRUST_L2_E :/*     trust UP (and not trust DSCP) */
            CPSS_QOS_PORT_NO_TRUST_E);/* not trust UP (and not trust DSCP) */
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosPortQosTrustModeSet");

        /* trust EXP mode */
        rc = prvTgfCosTrustExpModeSet(prvTgfPortsArray[testedPortIndex],
            (trustMode == CPSS_QOS_PORT_TRUST_L2_E || trustMode == CPSS_QOS_PORT_NO_TRUST_E) ?
            GT_FALSE : /* not trust EXP */
            GT_TRUE);  /*     trust EXP */
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosTrustExpModeSet");
    }
    else    /*ipv4,ipv6*/
    {
        /* trust UP,DSCP mode */
        rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[testedPortIndex],trustMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosPortQosTrustModeSet");

        /* trust EXP mode - NO trust */
        rc = prvTgfCosTrustExpModeSet(prvTgfPortsArray[testedPortIndex],GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfCosTrustExpModeSet");
    }

    PRV_UTF_LOG0_MAC("======= clear capture table =======\n");
    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[testedPortIndex],
        sentPacketInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
        prvTgfDevsArray[testedPortIndex], prvTgfBurstCount, 0, 0);

    PRV_UTF_LOG1_MAC("start Port [%d] capturing:\n", prvTgfDevsArray[testedPortIndex]);
    captureEgressTrafficOnPort(GT_TRUE/*start*/,capturedPortIndex);

    utfPrintKeepAlive();

    PRV_UTF_LOG1_MAC("======= send Packet from sender port[%d] =======\n",prvTgfPortsArray[testedPortIndex]);
    /* send Packet from sender port  */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[testedPortIndex],
        prvTgfPortsArray[testedPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth: %d %d",
        prvTgfDevsArray[testedPortIndex],
        prvTgfPortsArray[testedPortIndex]);

    PRV_UTF_LOG1_MAC("Stop Port [%d] capturing:\n", prvTgfDevsArray[testedPortIndex]);
    captureEgressTrafficOnPort(GT_FALSE/*end*/,capturedPortIndex);

    checkCaptureEgressTrafficOnPort(packetType,capturedPortIndex,sentPacketInfoPtr,
            expectedUp,expectedDscp,expectedExp);

    PRV_UTF_LOG1_MAC("End Port [%d] \n", prvTgfDevsArray[testedPortIndex]);
}

/**
* @internal tgfCosMappingTableIndexTest function
* @endinternal
*
* @brief   test 3 of the 12 mapping tables that the device hold.
*/
GT_VOID tgfCosMappingTableIndexTest
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  mappingTableIndex;
    GT_U32  testedPortIndex = 1;
    GT_U32  capturedPortIndex = 2;
    MAPPING_TABLE_INDEX_PACKET_TYPE_ENT packetType;
    GT_U32  trustModeIndex;
    CPSS_QOS_PORT_TRUST_MODE_ENT    trustMode;
    GT_U32  expectedEgressIndex;
    TGF_PACKET_STC* packetInfoPtr;
    GT_BOOL expectExpModify;/* do we expect change in EXP */
    GT_U8   expectExp;

    forceLinkUpOnAllTestedPorts();
    mappingTableIndexConfig(testedPortIndex,GT_TRUE/*start*/);

    PRV_UTF_LOG1_MAC("prvTgfBrgDefVlanEntryWrite(%d) \n",
        packetVid);
    rc = prvTgfBrgDefVlanEntryWrite(packetVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite");

    PRV_UTF_LOG4_MAC("prvTgfBrgVlanMemberAdd(%d,%d,%d,%d) \n",
        prvTgfDevNum,
        packetVid,
        prvTgfPortsArray[capturedPortIndex],
        GT_TRUE);
    /* set captured port as tagged (Tag0) member in vlan */
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum,packetVid,prvTgfPortsArray[capturedPortIndex],GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd");

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
        {
             expectExpModify = GT_FALSE;
        }
        else
        {
            expectExpModify = GT_TRUE;
        }
    }
    else
    {
        expectExpModify = GT_TRUE;
    }

    for(mappingTableIndex = 0 ; mappingTableIndex < MAPPING_TABLE_SIZE; mappingTableIndex++)
    {
        mappingTableIndexSet(mappingTableIndex,testedPortIndex);

        for(packetType = 0 ; packetType < MAPPING_TABLE_INDEX_PACKET_TYPE____LAST____E; packetType++)
        {
            packetInfoPtr = mappingTableIndexPacketsArr[packetType];

            for(trustModeIndex = 0; trustModeIndex < NUM_TRUST_MODE ;trustModeIndex++)
            {
                trustMode = mappingTableIndexTrustModeArr[trustModeIndex];

                expectedEgressIndex = (mappingTableIndex * 3);
                if(trustMode == CPSS_QOS_PORT_TRUST_L3_E)
                {
                    if(packetType == MAPPING_TABLE_INDEX_PACKET_TYPE_MPLS_E)
                    {
                        expectedEgressIndex += 2;/*according to EXP*/
                    }
                    else
                    {
                        expectedEgressIndex += 0;/*according to DSCP*/
                    }
                }
                else
                {
                    expectedEgressIndex += 1;/*according to UP*/
                }

                /* in legacy devices the EXP label is not modified unless 'LSR',
                    but SIP5 allows EXP modification in bridge MPLS as well. */
                expectExp = ((expectExpModify == GT_TRUE) ?
                             (GT_U8)cosQosTableInfoArr[expectedEgressIndex].entry.exp :
                             packetExp);
                sendPacketAndCheck(packetType,testedPortIndex,
                            capturedPortIndex,trustMode,packetInfoPtr,
                            (GT_U8)cosQosTableInfoArr[expectedEgressIndex].entry.userPriority,
                            (GT_U8)cosQosTableInfoArr[expectedEgressIndex].entry.dscp,
                            expectExp);
            }
        }
    }

    /************************/
    /*restore configurations*/
    /************************/
    PRV_UTF_LOG0_MAC(" ===== restoring ALL configurations ======== \n");

    mappingTableIndexConfig(testedPortIndex,GT_FALSE /*restore*/);

    rc = prvTgfBrgDefVlanEntryInvalidate(packetVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    PRV_UTF_LOG0_MAC(" ===== reset port's counters ======== \n");
    /* reset counter -- also do:
        clear table
        tgfTrafficTableRxPcktTblClear();
    */
    prvTgfEthCountersReset(prvTgfDevsArray[0]);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    PRV_UTF_LOG0_MAC(" ===== test Ended ======== \n");
}

static QOS_ENTRY_STC cosQosForPortProtocolArr[3]={
                                            /*dp,   up, tc, dscp,   exp*/
/*from per port*/      {105 /*qosProfile*/ , {0,    0,  0,  10,     1}}
/*from port protocol*/,{ 50 /*qosProfile*/ , {0,    4,  0,  11,     5}}
/*from tti action*/   ,{ 66 /*qosProfile*/ , {0,    7,  0,  12,     6}}
};

static QOS_ENTRY_STC origCosQosForPortProtocolArr[3]={
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME},    /* filled in runtime */
    {FILLED_IN_RUNTIME}     /* filled in runtime */
};

/* DSCP to use on the tested packet -ForPortProtocol */
static GT_U8   packetDscpForPortProtocol  = 50;
/* UP to use on the tested packet - ForPortProtocol*/
static GT_U8   packetUpForPortProtocol    = 5;
/* CFI to use on the tested packet - ForPortProtocol*/
static GT_U8   packetCfiForPortProtocol   = 0;
/* EXP to use on the tested packet - ForPortProtocol*/
static GT_U8   packetExpForPortProtocol   = 3;
/* the protocol index to use for the traffic types */
static GT_U32  portProtocolIndexArr[MAPPING_TABLE_INDEX_PACKET_TYPE____LAST____E] =
{
/* IPV4 */  9,
/* IPV6 */  3,
/* MPLS */  11
};


#define  PRECEDENCE_SOFT    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
#define  PRECEDENCE_HARD    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E
#define  PRECEDENCE_TYPE    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT

typedef enum{
    PORT_PROTOCOL_QOS_INDEX_SOURCE_PORT_E,
    PORT_PROTOCOL_QOS_INDEX_SOURCE_PROTOCOL_E,
    PORT_PROTOCOL_QOS_INDEX_SOURCE_TTI_ACTION_E,

    PORT_PROTOCOL_QOS_INDEX_SOURCE___NOT_VALID____E/* must be last */

}PORT_PROTOCOL_QOS_INDEX_SOURCE_ENT;
#define SOURCE_PER_PORT                 PORT_PROTOCOL_QOS_INDEX_SOURCE_PORT_E
#define SOURCE_PER_PORT_PER_PROTOCOL    PORT_PROTOCOL_QOS_INDEX_SOURCE_PROTOCOL_E
#define SOURCE_TTI_ACTION               PORT_PROTOCOL_QOS_INDEX_SOURCE_TTI_ACTION_E
#define SOURCE___NOT_VALID              PORT_PROTOCOL_QOS_INDEX_SOURCE___NOT_VALID____E
#define QOS_INDEX_SOURCE                PORT_PROTOCOL_QOS_INDEX_SOURCE_ENT

#define KEEP            CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
#define MODIFY_DISABLE  CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E
#define MODIFY_ENABLE   CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E
#define MODIFY_TYPE     CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT

typedef struct{
    QOS_INDEX_SOURCE    qosIndexSource;/*'port' / 'port protocol' / 'tti action' */
    PRECEDENCE_TYPE     qosPrecedence; /*soft/hard */
    MODIFY_TYPE         modifyDscpExp; /*keep/disable/enable*/
    MODIFY_TYPE         modifyUp;      /*keep/disable/enable*/
    GT_BOOL             portProtocolEnable__ttiKeepPreviousQos;/* for 'per port' indication to enable/disable the QOS from 'port protocol'.
                                                                  for TTI action 'keep previous QoS' */
}QOS_MODIFICATION_PARAM_STC;

#define TTI_NOT_KEEP_PREV   GT_FALSE
#define TTI_KEEP_PREV       GT_TRUE

typedef struct{
    QOS_MODIFICATION_PARAM_STC  arr[SOURCE___NOT_VALID];/*
                                        index is :
                                        SOURCE_PER_PORT,
                                        SOURCE_PER_PORT_PER_PROTOCOL,
                                        PORT_PROTOCOL_QOS_INDEX_SOURCE_TTI_ACTION_E,
                                            */
}PORT_PROTOCOL_QOS_MODIFICATION_PARAM_STC;

#define PORT_PROTOCOL_NUM_OF_TESTS_CNS  8

static PORT_PROTOCOL_QOS_MODIFICATION_PARAM_STC portProtocolModifyParam_TestConfig[PORT_PROTOCOL_NUM_OF_TESTS_CNS] =
{
    /*test #1 : check Qos precedence */
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   {{{ SOURCE_PER_PORT ,                PRECEDENCE_HARD , MODIFY_DISABLE ,  MODIFY_ENABLE  , GT_TRUE}
    ,{ SOURCE_PER_PORT_PER_PROTOCOL ,   PRECEDENCE_SOFT , MODIFY_ENABLE ,   MODIFY_DISABLE , GT_FALSE}
    ,{SOURCE___NOT_VALID}}
   }

    /*test #2 : check Qos precedence */
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   ,{{{ SOURCE_PER_PORT ,               PRECEDENCE_SOFT , MODIFY_DISABLE ,  MODIFY_ENABLE , GT_TRUE}
   ,{ SOURCE_PER_PORT_PER_PROTOCOL ,    PRECEDENCE_SOFT , MODIFY_ENABLE ,   MODIFY_DISABLE, GT_FALSE}
   ,{SOURCE___NOT_VALID}
   }}

    /*test #3 : check 'Protocol Based QoS Enable' = disabled */
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   ,{{{ SOURCE_PER_PORT ,               PRECEDENCE_SOFT , MODIFY_DISABLE ,  MODIFY_ENABLE , GT_FALSE}
   ,{ SOURCE_PER_PORT_PER_PROTOCOL ,    PRECEDENCE_SOFT , MODIFY_ENABLE ,   MODIFY_DISABLE, GT_FALSE}
   ,{SOURCE___NOT_VALID}
   }}

   /********************** tests with TTI action **************************/
    /*Test #4: (check TTI ignore Qos precedence , TTI action 'modify UP' keep)*/
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   ,{{{ SOURCE_PER_PORT ,               PRECEDENCE_HARD , MODIFY_DISABLE  ,  MODIFY_ENABLE, GT_TRUE}
   ,{ SOURCE_PER_PORT_PER_PROTOCOL ,    PRECEDENCE_HARD , MODIFY_ENABLE ,  MODIFY_DISABLE , GT_FALSE}
   ,{ SOURCE_TTI_ACTION ,               PRECEDENCE_SOFT , MODIFY_DISABLE, KEEP            , TTI_NOT_KEEP_PREV}
   }}

    /*Test #5: (check TTI ignore Qos precedence , TTI action 'modify DSCP/EXP' keep)*/
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   ,{{{ SOURCE_PER_PORT ,               PRECEDENCE_HARD , MODIFY_DISABLE  ,  MODIFY_ENABLE, GT_TRUE }
   ,{ SOURCE_PER_PORT_PER_PROTOCOL ,    PRECEDENCE_HARD , MODIFY_ENABLE ,  MODIFY_DISABLE , GT_FALSE }
   ,{ SOURCE_TTI_ACTION ,               PRECEDENCE_SOFT , KEEP/*KEEP*/          , MODIFY_ENABLE   , TTI_NOT_KEEP_PREV}
   }}

    /*Test #6: (check TTI keep previous qos index - (from per port))*/
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   ,{{{ SOURCE_PER_PORT ,               PRECEDENCE_HARD , MODIFY_DISABLE  ,  MODIFY_ENABLE, GT_TRUE }
   ,{ SOURCE_PER_PORT_PER_PROTOCOL ,    PRECEDENCE_HARD , MODIFY_ENABLE ,  MODIFY_DISABLE , GT_FALSE }
   ,{ SOURCE_TTI_ACTION ,               PRECEDENCE_SOFT , KEEP          , MODIFY_ENABLE   , TTI_KEEP_PREV}
   }}

    /*Test #7: (check TTI keep previous qos index - (from per protocol))*/
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   ,{{{ SOURCE_PER_PORT ,               PRECEDENCE_SOFT , MODIFY_DISABLE  ,  MODIFY_ENABLE, GT_TRUE }
   ,{ SOURCE_PER_PORT_PER_PROTOCOL ,    PRECEDENCE_HARD , MODIFY_ENABLE ,  MODIFY_DISABLE , GT_FALSE }
   ,{ SOURCE_TTI_ACTION ,               PRECEDENCE_SOFT , KEEP          , MODIFY_ENABLE   , TTI_KEEP_PREV}
   }}

    /*Test #8: (check TTI ignore Qos precedence , TTI action 'modify UP' keep - use port protocol)*/
    /*qosIndexSource                    qosPrecedence     modifyDscpExp     modifyUp        portProtocolEnable__ttiKeepPreviousQos*/
   ,{{{ SOURCE_PER_PORT ,               PRECEDENCE_SOFT , MODIFY_DISABLE  ,  MODIFY_ENABLE, GT_TRUE}
   ,{ SOURCE_PER_PORT_PER_PROTOCOL ,    PRECEDENCE_HARD , MODIFY_ENABLE ,  MODIFY_DISABLE , GT_FALSE}
   ,{ SOURCE_TTI_ACTION ,               PRECEDENCE_SOFT , MODIFY_DISABLE, KEEP            , TTI_NOT_KEEP_PREV}
   }}

};

typedef struct{
    QOS_INDEX_SOURCE    qosIndexSource;
    QOS_INDEX_SOURCE    modifyDscpExp;
    QOS_INDEX_SOURCE    modifyUp;
}PORT_PROTOCOL_RESULTS_STC;

static PORT_PROTOCOL_RESULTS_STC  portProtocolModifyParam_Test_results[PORT_PROTOCOL_NUM_OF_TESTS_CNS] =
{
    /*test #1 : check Qos precedence */
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
    {SOURCE_PER_PORT                ,SOURCE_PER_PORT                    ,SOURCE_PER_PORT    }
    /*test #2 : check Qos precedence */
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT_PER_PROTOCOL   ,SOURCE_PER_PORT_PER_PROTOCOL       ,SOURCE_PER_PORT_PER_PROTOCOL    }
    /*test #3 : check 'Protocol Based QoS Enable' = disabled */
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT                ,SOURCE_PER_PORT                    ,SOURCE_PER_PORT                 }

   /********************** tests with TTI action **************************/
    /*Test #4: (check TTI ignore Qos precedence , TTI action 'modify UP' keep)*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_TTI_ACTION              ,SOURCE_TTI_ACTION                  ,SOURCE_PER_PORT    }
    /*Test #5: (check TTI ignore Qos precedence , TTI action 'modify DSCP/EXP' keep)*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_TTI_ACTION              ,SOURCE_PER_PORT                    , SOURCE_TTI_ACTION   }
    /*Test #6: (check TTI keep previous qos index - (from per port))*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT               ,SOURCE_PER_PORT                     , SOURCE_TTI_ACTION   }
    /*Test #7: (check TTI keep previous qos index - (from per protocol))*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT_PER_PROTOCOL  ,SOURCE_PER_PORT_PER_PROTOCOL        , SOURCE_TTI_ACTION   }
    /*Test #8: (check TTI ignore Qos precedence , TTI action 'modify UP' keep - use port protocol)*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_TTI_ACTION              ,SOURCE_TTI_ACTION                  ,SOURCE_PER_PORT_PER_PROTOCOL    }

};

static PORT_PROTOCOL_RESULTS_STC  portProtocolModifyParamErrata_Test_results[PORT_PROTOCOL_NUM_OF_TESTS_CNS] =
{
    /*test #1 : check Qos precedence */
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
    {SOURCE_PER_PORT                ,SOURCE_PER_PORT                    ,SOURCE_PER_PORT    }
    /*test #2 : check Qos precedence */
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT_PER_PROTOCOL   ,SOURCE_PER_PORT_PER_PROTOCOL       ,SOURCE_PER_PORT_PER_PROTOCOL    }
    /*test #3 : check 'Protocol Based QoS Enable' = disabled */
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT                ,SOURCE_PER_PORT                    ,SOURCE_PER_PORT                 }

   /********************** tests with TTI action **************************/
    /*Test #4: (check TTI ignore Qos precedence , TTI action 'modify UP' keep)*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_TTI_ACTION              ,SOURCE_TTI_ACTION                  ,SOURCE_PER_PORT    }
    /*Test #5: (check TTI ignore Qos precedence , TTI action 'modify DSCP/EXP' keep)*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_TTI_ACTION              ,SOURCE_PER_PORT                    , SOURCE_TTI_ACTION   }
    /*Test #6: (check TTI keep previous qos index - (from per port))*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT               ,SOURCE_PER_PORT                     , SOURCE_TTI_ACTION   }
    /*Test #7: (check TTI keep previous qos index - (from per protocol))*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_PER_PORT_PER_PROTOCOL  ,SOURCE_PER_PORT_PER_PROTOCOL        ,SOURCE_PER_PORT_PER_PROTOCOL   }
    /*Test #8: (check TTI ignore Qos precedence , TTI action 'modify UP' keep - use port protocol)*/
    /*qosIndexSource                  modifyDscpExp                     modifyUp*/
   ,{SOURCE_TTI_ACTION              ,SOURCE_PER_PORT_PER_PROTOCOL       ,SOURCE_PER_PORT_PER_PROTOCOL    }

};

/* check that the expected 'modify' of UP or DSCP/EXP can be converted to GT_TRUE/GT_FALSE without the 'keep' value */
#define MODIFY_TYPE_TO_BOOL_CHECK(_modify,_enableModify)\
    if(_modify == MODIFY_DISABLE)          \
    {                                      \
        _enableModify = GT_FALSE;          \
    }                                      \
    else if(_modify == MODIFY_ENABLE)      \
    {                                      \
        _enableModify = GT_TRUE;           \
    }                                      \
    else                                   \
    {                                      \
        /*error*/                          \
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE, "MODIFY_TYPE_TO_BOOL_CHECK");\
        _enableModify = GT_TRUE;           \
    }

/* original PCL id for ethernet packets -- in the TTi lookup */
static GT_U32   origEthernetTtiPclId = 0;
/* tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           321
/* PCLID for the rule - for the ethernet packets */
#define PRV_TGF_TTI_ETHERNET_PCL_ID_CNS           0x2DB

/**
* @internal tunnelInit function
* @endinternal
*
* @brief   init tunnel termination relate to the test.
*
* @param[in] testId                   - test Id
* @param[in] senderPortIndex          - index of the sending port
*                                       None
*/
static GT_VOID tunnelInit
(
    IN  GT_U32  testId,
    IN GT_U32  senderPortIndex
)
{
    GT_STATUS   rc      ;
    PRV_TGF_TTI_RULE_UNT        pattern;
    PRV_TGF_TTI_RULE_UNT        mask;
    PRV_TGF_TTI_ACTION_2_STC      ruleAction;
    GT_U32              ii;

    rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,&origEthernetTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,PRV_TGF_TTI_ETHERNET_PCL_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* clear entry */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&ruleAction,0, sizeof(ruleAction));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate       = GT_FALSE;
    ruleAction.command         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand = PRV_TGF_TTI_NO_REDIRECT_E;
    ruleAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag1UpCommand  = PRV_TGF_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;

    /* set QOS related parameters*/
    ii = SOURCE_TTI_ACTION;

    ruleAction.qosProfile       = cosQosForPortProtocolArr[ii].qosProfile;
    ruleAction.qosPrecedence    = portProtocolModifyParam_TestConfig[testId].arr[ii].qosPrecedence;
    ruleAction.keepPreviousQoS  = portProtocolModifyParam_TestConfig[testId].arr[ii].portProtocolEnable__ttiKeepPreviousQos;

    switch(portProtocolModifyParam_TestConfig[testId].arr[ii].modifyUp)
    {
        case    CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            ruleAction.modifyTag0Up = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_UP_E;
            break;
        case    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            ruleAction.modifyTag0Up = PRV_TGF_TTI_MODIFY_UP_DISABLE_E;
            break;
        case    CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            ruleAction.modifyTag0Up = PRV_TGF_TTI_MODIFY_UP_ENABLE_E;
            break;
        default:
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE, "unknown type");

    }

    switch(portProtocolModifyParam_TestConfig[testId].arr[ii].modifyDscpExp)
    {
        case    CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            ruleAction.modifyDscp = PRV_TGF_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
            break;
        case    CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
            ruleAction.modifyDscp = PRV_TGF_TTI_MODIFY_DSCP_DISABLE_E;
            break;
        case    CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            ruleAction.modifyDscp = PRV_TGF_TTI_MODIFY_DSCP_ENABLE_E;
            break;
        default:
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE, "unknown type");

    }

    /* configure TTI rule */
    pattern.eth.common.pclId    = PRV_TGF_TTI_ETHERNET_PCL_ID_CNS;
    pattern.eth.common.srcIsTrunk = GT_FALSE;
    pattern.eth.common.srcPortTrunk = prvTgfPortsArray[senderPortIndex];

    mask.eth.common.pclId    = BIT_10 - 1;
    mask.eth.common.srcIsTrunk = 1;
    mask.eth.common.srcPortTrunk = BIT_7 - 1;

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                          &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}

static void portProtocolTestSet(
    IN  GT_U32  testId,
    IN  GT_U32  testedPortIndex,
    IN  MAPPING_TABLE_INDEX_PACKET_TYPE_ENT packetType
)
{
    GT_STATUS           rc;
    CPSS_QOS_ENTRY_STC  portQosParam;
    PRV_TGF_BRG_QOS_PROT_CLASS_CFG_STC     prvTgfQosCfg;
    GT_U32              ii;
    GT_U32  modify;
    GT_BOOL enableModify;
    GT_BOOL portProtocolEnabled;

    /***********************************/
    /* set the per port configurations */
    /***********************************/
    ii = SOURCE_PER_PORT;
    portQosParam.qosProfileId       = cosQosForPortProtocolArr[ii].qosProfile;
    portQosParam.assignPrecedence   = portProtocolModifyParam_TestConfig[testId].arr[ii].qosPrecedence;

    modify = portProtocolModifyParam_TestConfig[testId].arr[ii].modifyUp;
    MODIFY_TYPE_TO_BOOL_CHECK(modify,enableModify);
    portQosParam.enableModifyUp     = modify;

    modify = portProtocolModifyParam_TestConfig[testId].arr[ii].modifyDscpExp;
    MODIFY_TYPE_TO_BOOL_CHECK(modify,enableModify);
    portQosParam.enableModifyDscp   = modify;

    rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[testedPortIndex],&portQosParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

    portProtocolEnabled = portProtocolModifyParam_TestConfig[testId].arr[ii].portProtocolEnable__ttiKeepPreviousQos;
    rc = prvTgfBrgVlanPortProtoClassQosEnable(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],portProtocolEnabled);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortProtoClassQosEnable");

    /************************************************/
    /* set the per port per protocol configurations */
    /************************************************/
    ii = SOURCE_PER_PORT_PER_PROTOCOL;

    cpssOsMemSet(&prvTgfQosCfg, 0, sizeof(prvTgfQosCfg));

    prvTgfQosCfg.qosAssignCmd = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    prvTgfQosCfg.qosParams.dp = 0;  /*not relevant to DX devices */
    prvTgfQosCfg.qosParams.dscp = 0;/*not relevant to DX devices */
    prvTgfQosCfg.qosParams.exp = 0; /*not relevant to DX devices */
    prvTgfQosCfg.qosParams.tc = 0;  /*not relevant to DX devices */
    prvTgfQosCfg.qosParams.up = 0;  /*not relevant to DX devices */
    prvTgfQosCfg.qosParamsModify.modifyDp = GT_FALSE;
    prvTgfQosCfg.qosParamsModify.modifyTc = GT_FALSE;

    modify = portProtocolModifyParam_TestConfig[testId].arr[ii].modifyDscpExp;
    MODIFY_TYPE_TO_BOOL_CHECK(modify,enableModify);
    prvTgfQosCfg.qosParamsModify.modifyDscp = enableModify;
    prvTgfQosCfg.qosParamsModify.modifyExp = enableModify;

    modify = portProtocolModifyParam_TestConfig[testId].arr[ii].modifyUp;
    MODIFY_TYPE_TO_BOOL_CHECK(modify,enableModify);
    prvTgfQosCfg.qosParamsModify.modifyUp = enableModify;

    prvTgfQosCfg.qosPrecedence = portProtocolModifyParam_TestConfig[testId].arr[ii].qosPrecedence;
    prvTgfQosCfg.qosProfileId = cosQosForPortProtocolArr[ii].qosProfile;

    /* write port protocol VID and QOS assignment values for port 0 */
    rc = prvTgfBrgVlanPortProtoVlanQosSet(prvTgfDevNum, prvTgfPortsArray[testedPortIndex],
                                          portProtocolIndexArr[packetType], NULL, &prvTgfQosCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortProtoVlanQosSet: %d, ", prvTgfDevNum);


    /***********************/
    /* set the TTI action  */
    /***********************/
    ii = SOURCE_TTI_ACTION;

    if(portProtocolModifyParam_TestConfig[testId].arr[ii].qosIndexSource == SOURCE___NOT_VALID)
    {
        /* no TTI configuration needed */
        return;
    }

    tunnelInit(testId,testedPortIndex);

}

/* Init/restore global settings relate to the port protocol test */
static void portProtocolTestCofig(
    IN GT_U32            testedPortIndex,
    IN GT_BOOL           startOrRestore
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    TGF_PACKET_ETHERTYPE_STC    *etherTypePartPtr;
    CPSS_PROT_CLASS_ENCAP_STC   prvTgfEncList;
    MAPPING_TABLE_INDEX_PACKET_TYPE_ENT packetType;
    TGF_PACKET_STC* packetInfoPtr;

   TGF_PARAM_NOT_USED(testedPortIndex);

    if(startOrRestore == GT_TRUE)
    {
        prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

        prvTgfEncList.ethV2 = GT_TRUE;
        prvTgfEncList.llcSnap = GT_TRUE;
        prvTgfEncList.nonLlcSnap = GT_TRUE;

        for(ii = 0 ; ii < 3 ; ii++)
        {
            rc = prvTgfCosProfileEntryGet(prvTgfDevNum,cosQosForPortProtocolArr[ii].qosProfile,
                                &origCosQosForPortProtocolArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntryGet");

            rc = prvTgfCosProfileEntrySet(cosQosForPortProtocolArr[ii].qosProfile,&cosQosForPortProtocolArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");
        }

        for(packetType = 0 ; packetType < MAPPING_TABLE_INDEX_PACKET_TYPE____LAST____E; packetType++)
        {
            packetInfoPtr = mappingTableIndexPacketsArr[packetType];
            etherTypePartPtr = packetInfoPtr->partsArray[2/*ethertype part*/].partPtr;

            /* Write etherType and encapsulation of Protocol based classification */
            rc = prvTgfBrgVlanProtocolEntrySet(portProtocolIndexArr[packetType],
                                               etherTypePartPtr->etherType, &prvTgfEncList);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanProtocolEntrySet");
        }

        /* set COS parameters on the packets */
        prvTgfPacketVlanTag2Part.vid = packetVid;
        prvTgfPacketVlanTag2Part.pri = packetUpForPortProtocol;
        prvTgfPacketVlanTag2Part.cfi = packetCfiForPortProtocol;
        /* set ipv4 typeOfService according to dscp */
        prvTgfPacketIpv4OtherPart.typeOfService     = TGF_PACKET_IPV4_DSCP_TO_TYPE_OF_SERVICE_MAC(packetDscpForPortProtocol);
        /* set ipv6 trafficClass according to dscp */
        prvTgfPacketIpv6Part.trafficClass           = TGF_PACKET_IPV6_DSCP_TO_TRAFFIC_CLASS_MAC(packetDscpForPortProtocol);
        prvTgfPacket1MplsPart.exp = packetExpForPortProtocol;
    }
    else  /*restore*/
    {
        for(ii = 0 ; ii < 3 ; ii++)
        {
            rc = prvTgfCosProfileEntrySet(cosQosForPortProtocolArr[ii].qosProfile,&origCosQosForPortProtocolArr[ii].entry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfCosProfileEntrySet");
        }

        for(packetType = 0 ; packetType < MAPPING_TABLE_INDEX_PACKET_TYPE____LAST____E; packetType++)
        {
            /* invalidate etherType and encapsulation of Protocol based classification */
            rc = prvTgfBrgVlanProtocolEntryInvalidate(portProtocolIndexArr[packetType]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanProtocolEntryInvalidate");
        }

        /* disable port protocol QoS classification on the port */
        rc = prvTgfBrgVlanPortProtoClassQosEnable(prvTgfDevNum,prvTgfPortsArray[testedPortIndex],GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortProtoClassQosEnable");

        /* restore pcl-id for the etherenet traffic */
        rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,origEthernetTtiPclId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

        /* disable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[testedPortIndex],
                                          PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

        /* invalidate the TTI rule */
        rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS,GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

        prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
    }
}

static GT_U32   debug_tgfCosPortProtocolTestId = 0xFF;

/**
* @internal tgfCosPortProtocolTest function
* @endinternal
*
* @brief   test port protocol qos assignment
*/
GT_VOID tgfCosPortProtocolTest
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  testedPortIndex = 3;
    GT_U32  capturedPortIndex = 1;
    MAPPING_TABLE_INDEX_PACKET_TYPE_ENT packetType;
    CPSS_QOS_PORT_TRUST_MODE_ENT    trustMode = CPSS_QOS_PORT_NO_TRUST_E;
    TGF_PACKET_STC* packetInfoPtr;
    GT_U8   expectedUp;
    GT_U8   expectedDspc;
    GT_U8   expectedExp;
    GT_U32  testId;
    GT_U32  tblIndex1;/*index to portProtocolModifyParam_TestConfig[testId].Arr[]*/
    GT_U32  tblIndex2;/*index to cosQosForPortProtocolArr */
    GT_U32  modify;
    GT_BOOL enableModify;
    GT_BOOL debug = GT_FALSE;

    if(debug_tgfCosPortProtocolTestId < PORT_PROTOCOL_NUM_OF_TESTS_CNS)
    {
        debug = GT_TRUE;
    }

    forceLinkUpOnAllTestedPorts();
    portProtocolTestCofig(testedPortIndex,GT_TRUE/*init*/);

    PRV_UTF_LOG1_MAC("prvTgfBrgDefVlanEntryWrite(%d) \n",
        packetVid);
    rc = prvTgfBrgDefVlanEntryWrite(packetVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite");

    PRV_UTF_LOG4_MAC("prvTgfBrgVlanMemberAdd(%d,%d,%d,%d) \n",
        prvTgfDevNum,
        packetVid,
        prvTgfPortsArray[capturedPortIndex],
        GT_TRUE);
    /* set captured port as tagged (Tag0) member in vlan */
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum,packetVid,prvTgfPortsArray[capturedPortIndex],GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd");

    for(testId = 0 ; testId < PORT_PROTOCOL_NUM_OF_TESTS_CNS ; testId++)
    {
        if(debug == GT_TRUE)
        {
            testId = debug_tgfCosPortProtocolTestId;
        }

        /* add support for errata in BC2 => FE: TTI - 1265: Does not allow the TTI Action
           to set ModifyUP and/or ModifyDSCP if previously assigned with QoSPrecedence==HARD*/
        if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
        {
            tblIndex1 = portProtocolModifyParam_Test_results[testId].modifyDscpExp;
            tblIndex2 = portProtocolModifyParam_Test_results[testId].qosIndexSource;
        }
        else
        {

            if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                tblIndex1 = portProtocolModifyParamErrata_Test_results[testId].modifyDscpExp;
                tblIndex2 = portProtocolModifyParamErrata_Test_results[testId].qosIndexSource;
            }
            else
            {
                tblIndex1 = portProtocolModifyParam_Test_results[testId].modifyDscpExp;
                tblIndex2 = portProtocolModifyParam_Test_results[testId].qosIndexSource;
            }
        }

        modify = portProtocolModifyParam_TestConfig[testId].arr[tblIndex1].modifyDscpExp;
        MODIFY_TYPE_TO_BOOL_CHECK(modify,enableModify);

        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E))
        {
            /* in legacy devices the EXP label is not modified unless 'LSR'*/
            expectedExp = (GT_U8)packetExpForPortProtocol;
        }
        else /* Sip5 */
        {
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                 expectedExp = (GT_U8)packetExpForPortProtocol;
            }
            else
            {
                if(enableModify == GT_FALSE)
                {
                    /* the <EXP> not expected to be changed */
                    expectedExp = (GT_U8)packetExpForPortProtocol;
                }
                else
                {
                    /* SIP5 allows EXP modification in bridge MPLS as well. (not only 'LSR') */
                    expectedExp = (GT_U8)cosQosForPortProtocolArr[tblIndex2].entry.exp;
                }
            }
        }

        if(enableModify == GT_FALSE)
        {
            /* the <DSCP> not expected to be changed */
            expectedDspc = (GT_U8)packetDscpForPortProtocol;
        }
        else
        {
            expectedDspc = (GT_U8)cosQosForPortProtocolArr[tblIndex2].entry.dscp;
        }

        /* add support for errata in BC2 => FE: TTI - 1265: Does not allow the TTI Action
           to set ModifyUP and/or ModifyDSCP if previously assigned with QoSPrecedence==HARD*/
        if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
        {
            tblIndex1 = portProtocolModifyParam_Test_results[testId].modifyUp;
        }
        else
        {
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                tblIndex1 = portProtocolModifyParamErrata_Test_results[testId].modifyUp;
            }
            else
            {
                tblIndex1 = portProtocolModifyParam_Test_results[testId].modifyUp;
            }
        }

        modify = portProtocolModifyParam_TestConfig[testId].arr[tblIndex1].modifyUp;
        MODIFY_TYPE_TO_BOOL_CHECK(modify,enableModify);

        if(enableModify == GT_FALSE)
        {
            /* the <UP> not expected to be changed */
            expectedUp = (GT_U8)packetUpForPortProtocol;
        }
        else
        {
            expectedUp   = (GT_U8)cosQosForPortProtocolArr[tblIndex2].entry.userPriority;
        }

        for(packetType = 0 ; packetType < MAPPING_TABLE_INDEX_PACKET_TYPE____LAST____E; packetType++)
        {
            packetInfoPtr = mappingTableIndexPacketsArr[packetType];

            portProtocolTestSet(testId,testedPortIndex,packetType);

            PRV_UTF_LOG5_MAC("tgfCosPortProtocolTest : (testId[%d],packetType[%d],expectedUp[%d],expectedDspc[%d],expectedExp[%d]) \n" ,
                                testId ,
                                packetType,
                                expectedUp,expectedDspc,expectedExp);


            sendPacketAndCheck(packetType,testedPortIndex,
                        capturedPortIndex,trustMode,packetInfoPtr,
                        expectedUp,expectedDspc,expectedExp);

            if(debug == GT_TRUE)
            {
                break;
            }
        }

        if(debug == GT_TRUE)
        {
            break;
        }

    }

    /************************/
    /*restore configurations*/
    /************************/
    PRV_UTF_LOG0_MAC(" ===== restoring ALL configurations ======== \n");

    portProtocolTestCofig(testedPortIndex,GT_FALSE /*restore*/);

    rc = prvTgfBrgDefVlanEntryInvalidate(packetVid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    PRV_UTF_LOG0_MAC(" ===== reset port's counters ======== \n");
    /* reset counter -- also do:
        clear table
        tgfTrafficTableRxPcktTblClear();
    */
    prvTgfEthCountersReset(prvTgfDevsArray[0]);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    PRV_UTF_LOG0_MAC(" ===== test Ended ======== \n");
}
/*debug tool*/
GT_STATUS debug_tgfCosPortProtocolTestIdSet(IN GT_U32  testId)
{
    debug_tgfCosPortProtocolTestId = testId;

    return GT_OK;
}

