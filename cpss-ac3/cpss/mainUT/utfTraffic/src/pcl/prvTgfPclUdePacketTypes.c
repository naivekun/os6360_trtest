/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPclUdePktTypes.c
*
* DESCRIPTION:
*       Ingress PCL Uset Defined Ethernet types using
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclTrunkHashVal.h>
#include <pcl/prvTgfPclUdePacketTypes.h>

/* amount of Ude Packet types */
#define PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS 5

/* Ude Packet type base */
#define PRV_TGF_PCL_UDE_PKT_TYPE_BASE_CNS 0x2201

/* Ude Packet L2 0ffset base */
#define PRV_TGF_PCL_UDE_L2_OFFSET_BASE_CNS 20

/* Ude Packet L2 0ffset base */
#define PRV_TGF_PCL_UDE_L2_PACKET_BYTE_BASE_CNS 0x70

/* Ude Packet key UDB index */
#define PRV_TGF_PCL_UDE_UDB_INDEX_CNS 15

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    sizeof(prvTgfPacketPayloadLongDataArr), /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x56},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOthEtherTypePart =
{0x3456};

/* VLAN tag1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_PCL_TEST_UP0  /*pri*/,
    PRV_TGF_PCL_TEST_CFI0 /*cfi*/,
    PRV_TGF_PCL_TEST_VID0 /*vid*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfEthOthPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOthEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of ETH_OTHER packet */
#define PRV_TGF_ETH_OTHER_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS \
    + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadLongDataArr)

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfEthOtherPacketInfo =
{
    PRV_TGF_ETH_OTHER_PACKET_LEN_CNS,              /* totalLen */
    (sizeof(prvTgfEthOthPacketPartArray)
        / sizeof(prvTgfEthOthPacketPartArray[0])), /* numOfParts */
    prvTgfEthOthPacketPartArray                    /* partsArray */
};

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfPclUdePktTypesVlanConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclUdePktTypesVlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG1_MAC(
        "\nVLAN %d CONFIGURATION:\n",
        PRV_TGF_PCL_TEST_VID0);
    PRV_UTF_LOG4_MAC("  Port members: [%d], [%d], [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1],
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 5 with all ports with TAG0 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_TEST_VID0,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        /* AUTODOC: create 5 VLANs [6..10] with all ports with TAG0 cmd */
        rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
            (GT_U16)(PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i),
            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);
    }

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

    /* AUTODOC: set PVID 5 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        PRV_TGF_PCL_TEST_VID0);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);
}

/**
* @internal prvTgfPclUdePktTypesConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclUdePktTypesConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable ingress policy on port 1 */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        /* AUTODOC: invalidate PCL rules */
        rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, i, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
            prvTgfDevNum, GT_TRUE);
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
        prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_PCL_TEST_VID0);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_PCL_TEST_VID0);

    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(
            (GT_U16)(PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i));
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, (PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i));
    }

    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        /* AUTODOC: restore default UDE EtherTypes */
        rc = prvTgfPclUdeEtherTypeSet(
            i, 0xFFFF);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclUdeEtherTypeSet: %d, %d",
            prvTgfDevNum, i);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclUdePktTypesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] packetIndex              - the index of the sent packet
*                                       None
*/
static GT_VOID prvTgfPclUdePktTypesTrafficGenerate
(
    IN GT_U32 packetIndex
)
{
    GT_STATUS            rc         = GT_OK;
    TGF_VFD_INFO_STC     vfdArray[2];
    GT_U16               ethType;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    ethType = (GT_U16)(PRV_TGF_PCL_UDE_PKT_TYPE_BASE_CNS + packetIndex);

    /* L2 offseted byte to be matched by UDB configuration */
    /* of an appropriate UDB                               */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset =
        (PRV_TGF_PCL_UDE_L2_OFFSET_BASE_CNS + packetIndex);
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] =
        (GT_U8)(PRV_TGF_PCL_UDE_L2_PACKET_BYTE_BASE_CNS + packetIndex);

    /* ethernet type */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].offset = 16; /* 6 + 6 + 4 */
    vfdArray[1].cycleCount = 2;
    vfdArray[1].patternPtr[0] = (GT_U8)((ethType >> 8) & 0xFF);
    vfdArray[1].patternPtr[1] = (GT_U8)(ethType & 0xFF);

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        1      /*burstCount*/,
        (sizeof(vfdArray) / sizeof(vfdArray[0])),
        vfdArray,
        prvTgfDevsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture: %d",
        prvTgfDevNum);
}

/**
* @internal prvTgfPclUdePktTypesTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] portIndex                - port index
* @param[in] packetIndex              - the index of the sent packet
*                                       None
*/
static GT_VOID prvTgfPclUdePktTypesTrafficEgressVidCheck
(
    IN GT_U32  portIndex,
    IN GT_U32  packetIndex
)
{
    GT_U16 egressVid;

    egressVid = (GT_U16)(PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + packetIndex);

    prvTgfPclTrunkHashTrafficEgressVidCheck(
        portIndex, egressVid);
}

/**
* @internal prvTgfPclUdePktTypesPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
static GT_VOID prvTgfPclUdePktTypesPclConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           i;
    PRV_TGF_PCL_OVERRIDE_UDB_STC     udbOverride;
    PRV_TGF_PCL_PACKET_TYPE_ENT      packetType;

    ruleIndex                   = 0;
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* FIX for lion_B0: smem 0b800014 MUST be 00000000 */
    cpssOsMemSet(&udbOverride,  0, sizeof(udbOverride));
    prvTgfPclOverrideUserDefinedBytesSet(&udbOverride);

    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        /* AUTODOC: set UDE Ethertype to [0x2201..0x2205]*/
        rc = prvTgfPclUdeEtherTypeSet(
            i, (PRV_TGF_PCL_UDE_PKT_TYPE_BASE_CNS + i));
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclUdeEtherTypeSet: %d, %d",
            prvTgfDevNum, i);
    }

    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        packetType =
            (i == 0)
                ? PRV_TGF_PCL_PACKET_TYPE_UDE_E
                : (PRV_TGF_PCL_PACKET_TYPE_UDE_1_E + (i - 1));

        /* AUTODOC: configure 5 User Defined Bytes (UDBs): */
        /* AUTODOC:   format INGRESS_STD_NOT_IP */
        /* AUTODOC:   packet type UDE */
        /* AUTODOC:   offsetType OFFSET_L2, offsets [20..24] */
        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType,
            CPSS_PCL_DIRECTION_INGRESS_E,
            PRV_TGF_PCL_UDE_UDB_INDEX_CNS,
            PRV_TGF_PCL_OFFSET_L2_E,
            (GT_U8)(PRV_TGF_PCL_UDE_L2_OFFSET_BASE_CNS + i));
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d, %d",
            prvTgfDevNum, i);
    }

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup_0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

    /* set PCL rule 0 - CMD_DROP_HARD packet 1 - any Not Ipv4 with MAC DA = ... 34 02 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* UDB15 */
    mask.ruleStdNotIp.udb[15]    = 0xFF;

    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    /* vlanCmd - for ExMxPm, modifyVlan for DxCh */
    action.vlan.vlanCmd         = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.modifyVlan      = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.precedence      =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        action.vlan.vlanId =
         (GT_U16)(PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i);

        pattern.ruleStdNotIp.udb[15] =
            (GT_U8)(PRV_TGF_PCL_UDE_L2_PACKET_BYTE_BASE_CNS + i);

        /* AUTODOC: config 5 PCL rules [0..4] with: */
        /* AUTODOC:   format - INGRESS_STD_NOT_IP */
        /* AUTODOC:   cmd FORWARD */
        /* AUTODOC:   pattern UDBs=[0x70..0x74] */
        rc = prvTgfPclRuleSet(
            ruleFormat, (ruleIndex + i), &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                     prvTgfDevNum, ruleFormat, ruleIndex);
    }
}


/**
* @internal prvTgfPclUdePktTypesTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic and check results
*/
static GT_VOID prvTgfPclUdePktTypesTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_U32             i;

    /* AUTODOC: GENERATE TRAFFIC: */
    for (i = 0; (i < PRV_TGF_PCL_UDE_PKT_TYPES_AMOUNT_CNS); i++)
    {
        /* AUTODOC: send 5 UDE packets on port 1 with: */
        /* AUTODOC:   DA=00:11:22:33:44:56, SA=00:66:77:88:99:00 */
        /* AUTODOC:   VID=5, EtherTypes=[0x2201..0x2205] */
        /* AUTODOC:   [20..24] bytes for single packet [0x70..0x74] */
        prvTgfPclUdePktTypesTrafficGenerate(i);

        /* AUTODOC: verify traffic on port 2: */
        /* AUTODOC:   expected 5 packets with VIDs=[6..10] in VLAN tag */
        prvTgfPclUdePktTypesTrafficEgressVidCheck(
            PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS, i);
    }
}

/**
* @internal prvTgfPclUdePktTypesTrafficTest function
* @endinternal
*
* @brief   Full UDE Packet Types test
*/
GT_VOID prvTgfPclUdePktTypesTrafficTest
(
    GT_VOID
)
{
    prvTgfPclUdePktTypesVlanConfigurationSet();

    prvTgfPclUdePktTypesPclConfigurationSet();

    prvTgfPclUdePktTypesTrafficGenerateAndCheck();

    prvTgfPclUdePktTypesConfigurationRestore();
}

