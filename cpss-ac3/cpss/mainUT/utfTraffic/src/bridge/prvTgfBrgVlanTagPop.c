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
* @file prvTgfBrgVlanTagPop.c
*
* @brief VLAN Tag words Pop.
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgVlanTagPop.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
#define BUILD_U32_FROM_TAG_MAC(tag)  \
    (tag.etherType << 16 |           \
     tag.pri << 13       |           \
     tag.cfi << 12       |           \
     tag.vid)

/* first VLAN Id */
#define PRV_TGF_VLANID_0_CNS          2

/* second VLAN Id */
#define PRV_TGF_VLANID_1_CNS          3

/* third VLAN Id */
#define PRV_TGF_VLANID_2_CNS          4

/* third VLAN Id */
#define PRV_TGF_VLANID_3_CNS          5

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS  1

/* port number to send vlan 2 traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS  2

/* Source ePort */
#define PRV_TGF_SRC_EPORT_CNS               ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1432)

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* PCL id for restore procedure */
static GT_U32 savePclId;
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_0_CNS                          /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    1, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag2Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    2, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag3Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    3, 0, PRV_TGF_VLANID_3_CNS                          /* pri, cfi, VlanId */
};

/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    0x3456
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag3Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    (TGF_L2_HEADER_SIZE_CNS
        + (4 * TGF_VLAN_TAG_SIZE_CNS)
        + TGF_ETHERTYPE_SIZE_CNS
        + sizeof(prvTgfPayloadDataArr)),                               /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanTagPopPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfBrgVlanTagPopPortVlanFdbSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in Passanger with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VLANID_0_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as TAG in Tunnel with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VLANID_1_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as PCL assigned VID with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VLANID_2_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as PCL assigned VID with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VLANID_3_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_0_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_1_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_2_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_3_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return GT_OK;
}

/**
* @internal prvTgfBrgVlanTagPopPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfBrgVlanTagPopPortVlanFdbReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset")

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear")

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgFdbFlush")

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Passanger */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_0_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryInvalidate")

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Tunnel */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_1_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryInvalidate")

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryInvalidate")

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_3_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryInvalidate")

    return rc1;
}

/**
* @internal prvTgfBrgVlanTagPopTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
static GT_VOID prvTgfBrgVlanTagPopTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal testTagsAfterPop function
* @endinternal
*
* @brief   Checks traffic egress Tags
*
* @param[in] egressTagsPtr            - pointer to egress tags
* @param[in] numOfTags                - number of egress tags
* @param[in] etherType                - the ethertype after the tags
*                                       None
*/
static GT_VOID testTagsAfterPop(
    IN GT_U32  *egressTagsPtr,
    IN GT_U32  numOfTags,
    IN GT_U16  etherType
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[9/*8+1*/];/*one more for ether type*/
    GT_U32                          numTriggersBmp;
    GT_U32                          ii;

    if(numOfTags > 8)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, GT_FULL, "testTagsAfterPop");
        return;
    }

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set vfd for destination MAC */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

    ii = 0;

    for(ii = 0 ; ii < numOfTags ; ii++)
    {
        vfdArray[ii].offset = 12 + ii*4; /* after mac addresses  */
        vfdArray[ii].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[ii].cycleCount = 4;
        vfdArray[ii].patternPtr[0] = (GT_U8)((egressTagsPtr[ii] >> 24) & 0xFF);
        vfdArray[ii].patternPtr[1] = (GT_U8)((egressTagsPtr[ii] >> 16) & 0xFF);
        vfdArray[ii].patternPtr[2] = (GT_U8)((egressTagsPtr[ii] >>  8) & 0xFF);
        vfdArray[ii].patternPtr[3] = (GT_U8)((egressTagsPtr[ii] >>  0) & 0xFF);
    }

    /* one for ethertype */
    vfdArray[ii].offset = 12 + ii*4; /* after mac addresses  */
    vfdArray[ii].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[ii].cycleCount = 2;
    vfdArray[ii].patternPtr[0] = (GT_U8)((etherType >>  8) & 0xFF);
    vfdArray[ii].patternPtr[1] = (GT_U8)((etherType >>  0) & 0xFF);

    ii++;

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            ii /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(
        ((1<<ii)- 1), numTriggersBmp, "NOT all [%d] triggers are matched \n",
        ii);

}

/**
* @internal prvTgfBrgVlanTagPopTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfBrgVlanTagPopTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, assign source ePort */
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.sourceEPortAssignmentEnable       = GT_TRUE;
    ttiAction.sourceEPort                       = PRV_TGF_SRC_EPORT_CNS;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TTI TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    ttiMask.eth.common.pclId    = 0x3FF;
    ttiPattern.eth.common.pclId = PRV_TGF_TTI_ID_CNS;

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, tunnelPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, tunnelPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS , GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEPortIngressPolicyEnable");

    return GT_OK;
}

/**
* @internal prvTgfBrgVlanTagPopTtiConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfBrgVlanTagPopTtiConfigReset
(
    GT_VOID
)
{
    GT_STATUS   rc, rc1 = GT_OK;
    PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_FALSE);
    PRV_UTF_VERIFY_LOG_NO_STOP_MAC(
        rc1, rc, "prvTgfTtiPortLookupEnableSet")

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_LOG_NO_STOP_MAC(
        rc1, rc, "prvTgfTtiRuleValidStatusSet")

    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS , GT_FALSE);
    PRV_UTF_VERIFY_LOG_NO_STOP_MAC(
        rc1, rc, "prvTgfPclEPortIngressPolicyEnable")

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfBrgVlanTagPopTtiTest function
* @endinternal
*
* @brief   Test on VLAN Tag Ingress Pop 0,4,8 Tag bytes.
*         The correct TTI removing.
* @param[in] numBytesToPop            - enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
* @param[in] withTtiActionReassignSrcEport - indication that need TTI action that
*                                      will do re-assign src EPort
*                                      (when GT_FALSE - meaning that the device will also
*                                      not do 'L2 reparse' so the popped tags are those
*                                      AFTER tag0,tag1)
*                                       None
*/
GT_VOID prvTgfBrgVlanTagPopTtiTest
(
    IN PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_ENT   numBytesToPop,
    IN GT_BOOL  withTtiActionReassignSrcEport
)
{
    GT_STATUS rc;
    GT_U32  egressTagsArr[4];/*the expected egress tags */
    GT_U32  srcEPort = 0xFFFF;

    GT_U16  errorExpectedOn2Tags = GT_FALSE;

    GT_U32  ingressPacketLength;/*length of ingress packet (including CRC) */
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32  tag0 = BUILD_U32_FROM_TAG_MAC(prvTgfPacketVlanTag0Part);
    GT_U32  tag1 = BUILD_U32_FROM_TAG_MAC(prvTgfPacketVlanTag1Part);
    GT_U32  tag2 = BUILD_U32_FROM_TAG_MAC(prvTgfPacketVlanTag2Part);
    GT_U32  tag3 = BUILD_U32_FROM_TAG_MAC(prvTgfPacketVlanTag3Part);
    GT_CHAR*    namesPtr[] = {"pop-none","pop-4-bytes","pop-8-bytes"};
    GT_U32  numBytesNumeric;/* number of bytes to pop (numeric ... not ENUM) */

    if(numBytesToPop >= PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_6_E)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, GT_FAIL, "prvTgfBrgVlanTagPopTtiTest: %d > 'pop_6' not supported",
            numBytesToPop);
        return;
    }

    numBytesNumeric =
        (numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E) ? 0 :
        (numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_4_E) ? 4 :
        (numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_8_E) ? 8 : 0;

    if(withTtiActionReassignSrcEport == GT_TRUE)
    {
        PRV_UTF_LOG1_MAC("start test [%s] tags with TTI action + L2 re-parsing for tags after the pop \n",
            namesPtr[numBytesToPop]);
    }
    else
    {
        PRV_UTF_LOG1_MAC("start test [%s] tags without TTI action so no L2 re-parsing \n",
            namesPtr[numBytesToPop]);
    }

    if(numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_8_E)
    {
        if(GT_TRUE == prvUtfIsGmCompilation())
        {
            errorExpectedOn2Tags = GT_FALSE;
        }
        else
        {
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                /* FE: HA-3374 - HA build wrong tags when <num_of_tags_to_pop> = 2 */
                errorExpectedOn2Tags = GT_TRUE;
            }
        }
    }

    /* AUTODOC: set VLANs, FDB, Capture */
    rc = prvTgfBrgVlanTagPopPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    if(withTtiActionReassignSrcEport == GT_TRUE)
    {
        /* AUTODOC: TTI rule to set source ePort */
        rc = prvTgfBrgVlanTagPopTtiConfigSet();
        if (rc != GT_OK) goto label_restore;

        srcEPort = PRV_TGF_SRC_EPORT_CNS;
    }
    else
    {
        srcEPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    }

    /* AUTODOC: set needed number of Tag Bytes to Pop */
    rc = prvTgfBrgVlanPortNumOfTagWordsToPopSet(
        prvTgfDevNum, srcEPort, numBytesToPop);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortNumOfTagWordsToPopSet: %s",namesPtr[numBytesToPop]);

    /* reset egress port counters before sending packet */
    rc = prvTgfResetCountersEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfResetCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* AUTODOC: send packet */
    prvTgfBrgVlanTagPopTrafficGenerate(&prvTgfTaggedPacketInfo);

    /* calculate the number of bytes in the packet that we sent */
    rc = tgfTrafficEnginePacketLengthCalc(&prvTgfTaggedPacketInfo,&ingressPacketLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficEnginePacketLengthCalc: rc = %d",
                                 rc);

    /* read egress port counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
    /*check that single packet egress the proper port */
    UTF_VERIFY_EQUAL1_STRING_MAC(1, portCntrs.goodPktsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d",
                            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* FE: HA-3374 - HA build wrong tags when <num_of_tags_to_pop> = 2 */
    if(errorExpectedOn2Tags == GT_TRUE)
    {
        /* via egress port counters : check that the packet was added 2 tags instead of losing 2 tags !!!
            so packet egress 6 tags instead of only 2 */
        UTF_VERIFY_EQUAL1_STRING_MAC((ingressPacketLength + 8), portCntrs.goodOctetsRcv.l[0],
                     "get another goodPktsRcv counter than expected on port %d",
                                prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
    }
    else
    {
        /* we expect to remove x tags that are 4x bytes */
        UTF_VERIFY_EQUAL1_STRING_MAC((ingressPacketLength - numBytesNumeric), portCntrs.goodOctetsRcv.l[0],
                     "get another goodPktsRcv counter than expected on port %d",
                                prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    }

    /* FE: HA-3374 - HA build wrong tags when <num_of_tags_to_pop> = 2 */
    if(errorExpectedOn2Tags == GT_TRUE)
    {
        /* AUTODOC: expect error in behavior of device due to errata */
        GT_U32          packetLen = 512;
        GT_U32          origPacketLen;
        GT_U8           devNum;
        GT_U8           queue;
        TGF_NET_DSA_STC rxParam;
        static GT_U8    packetBuf[512];
        CPSS_INTERFACE_INFO_STC         portInterface;

        cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

        portInterface.type = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        /* check that packet was not received at the CPU because the device not able to handle packet with 6 tags */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                            TGF_PACKET_TYPE_CAPTURE_E,
                                           GT_TRUE /*getFirst*/, GT_TRUE/*trace*/, packetBuf,
                                           &packetLen, &origPacketLen,
                                           &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_NO_MORE, rc,
            "ERROR : we expect to NOT get packet in the CPU (because device can not handle 6 tags) \n");
    }
    else
    {
        if(withTtiActionReassignSrcEport == GT_TRUE)
        {
            if(numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E)
            {
                egressTagsArr[0] = tag0;
                egressTagsArr[1] = tag1;
                egressTagsArr[2] = tag2;
                egressTagsArr[3] = tag3;
            }
            else if (numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_4_E)
            {
                /* first tag popped */
                egressTagsArr[0] = tag1;
                egressTagsArr[1] = tag2;
                egressTagsArr[2] = tag3;
                egressTagsArr[3] = 0;/* this is ignored */
            }
            else /*if (numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_8_E)*/
            {
                /* first tag popped , second tag popped */
                egressTagsArr[0] = tag2;
                egressTagsArr[1] = tag3;
                egressTagsArr[2] = 0;/* this is ignored */
                egressTagsArr[3] = 0;/* this is ignored */
            }
        }
        else
        {
            /* NOTE: this is strange behavior because the TTI action should be involved for proper behavior */

            egressTagsArr[0] = tag0;
            egressTagsArr[1] = tag1;

            if(numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E)
            {
                egressTagsArr[2] = tag2;
                egressTagsArr[3] = tag3;
            }
            else if (numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_4_E)
            {
                /* third tag popped */
                egressTagsArr[2] = tag3;
                egressTagsArr[3] = 0;/* this is ignored */
            }
            else /*if (numBytesToPop == PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_8_E)*/
            {
                /* third tag popped , fourth tag popped */
                egressTagsArr[2] = 0;/* this is ignored */
                egressTagsArr[3] = 0;/* this is ignored */
            }
        }

        /* we send 4 tags the pop of 0/4/8 bytes will leave us with 4/3/2 tags */
        testTagsAfterPop(&egressTagsArr[0],
                         4 - (numBytesNumeric / 4),
                         prvTgfPackeEthertypePart.etherType);
    }

    label_restore:

    if(srcEPort != 0xFFFF)
    {
        /* AUTODOC: restore to 0 Tag Bytes to Pop */
        rc = prvTgfBrgVlanPortNumOfTagWordsToPopSet(
            prvTgfDevNum, srcEPort, PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortNumOfTagWordsToPopSet");
    }

    if(withTtiActionReassignSrcEport == GT_TRUE)
    {
        /* AUTODOC: TTI rule to set source ePort */
        prvTgfBrgVlanTagPopTtiConfigReset();
    }

    /* AUTODOC: reset VLANs, FDB, Capture */
    prvTgfBrgVlanTagPopPortVlanFdbReset();
}



