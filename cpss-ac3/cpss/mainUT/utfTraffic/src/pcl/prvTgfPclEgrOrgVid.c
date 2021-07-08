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
* @file prvTgfPclEgrOrgVid.c
*
* @brief Ingress PCL Double tagged packet Egress Pcl Original VID field
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <common/tgfPacketGen.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <pcl/prvTgfPclTrunkHashVal.h>

/* VID for internal tag */
#define PRV_TGF_PCL_TEST_VID1 20

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x54

/* THE GM hold several BUGs !!!
1. that even if the EPCL drops the packet still it is
   counted at the egress MAC countes as 'goodPktsSent' !!! .

   so AS WA , I set here mac loopback on the egress port and check if the
   INGRESS of that port got packets or not .

2. the multiple egress ports cause the GM to swap descriptors between
   egress ports !!! so the DROP may happen on wrong packet that egress
   wrong port !!!! */
/* so to avoid swaps ... make sure only single port is in the */
static GT_BOOL needEpclWaOnDrop = GT_FALSE;

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

/* VLAN tag0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_PCL_TEST_UP0  /*pri*/,
    PRV_TGF_PCL_TEST_CFI0 /*cfi*/,
    PRV_TGF_PCL_TEST_VID0 /*vid*/
};

/* VLAN tag0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    PRV_TGF_PCL_TEST_UP0  /*pri*/,
    PRV_TGF_PCL_TEST_CFI0 /*cfi*/,
    PRV_TGF_PCL_TEST_VID1 /*vid*/
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
    TGF_L2_HEADER_SIZE_CNS + (TGF_VLAN_TAG_SIZE_CNS * 2) \
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
* @internal prvTgfPclEgrOrgVidVlanConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_VOID prvTgfPclEgrOrgVidVlanConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      i;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG1_MAC(
        "\nVLAN %d CONFIGURATION:\n",
        PRV_TGF_PCL_TEST_VID0);
    PRV_UTF_LOG4_MAC("  Port members: [%d], [%d], [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1],
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 5 with all ports with TAG1 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_TEST_VID0,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 20 with all ports with OUTER_TAG0_INNER_TAG1 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_TEST_VID1,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    for (i = 0; (i < 2); i++)
    {
        /* AUTODOC: create VLANs 6,7 with all ports with OUTER_TAG0_INNER_TAG1 cmd */
        rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
            (GT_U16)(PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i),
            PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
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
* @internal prvTgfPclEgrOrgVidConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
static GT_VOID prvTgfPclEgrOrgVidConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      i;
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum     =
        prvTgfDevsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS];
    interfaceInfo.devPort.portNum    =
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS];

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.lookupType             = PRV_TGF_PCL_LOOKUP_TYPE_SINGLE_E;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.externalLookup         = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key =
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table */
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    for (i = 0; (i < 2); i++)
    {
        /* AUTODOC: invalidate PCL rules */
        rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(i), GT_FALSE);
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

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(
        PRV_TGF_PCL_TEST_VID1);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
        prvTgfDevNum, PRV_TGF_PCL_TEST_VID1);

    for (i = 0; (i < 2); i++)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(
            (GT_U16)(PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i));
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, (PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i));
    }
}

/**
* @internal prvTgfPclEgrOrgVidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfPclEgrOrgVidTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: send Ethernet packet on port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:56, SA=00:66:77:88:99:00 */
    /* AUTODOC:   TAG0 VID=5, TAG1 VID=20, EtherType=0x3456 */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        &prvTgfEthOtherPacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture: %d",
        prvTgfDevNum);
}

/**
* @internal prvTgfPclEgrOrgVidTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] portIndex                - port index
* @param[in] packetIndex              - the index of the sent packet
*                                       None
*/
static GT_VOID prvTgfPclEgrOrgVidTrafficEgressVidCheck
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
* @internal prvTgfPclEgrOrgVidPclConfigurationSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
static GT_VOID prvTgfPclEgrOrgVidPclConfigurationSet
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

    ruleIndex                   = 0;
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;


    /* AUTODOC: init PCL Engine for send port 2: */
    /* AUTODOC:   egress direction, lookup_0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key EGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

    /* set PCL rule 0 - CMD_DROP_HARD packet 1 - any Not Ipv4 with MAC DA = ... 34 02 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* original VID */
    mask.ruleEgrStdNotIp.common.vid    = 0xFFF;

    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    /* vlanCmd - for ExMxPm, modifyVlan for DxCh */
    action.vlan.egressVlanId0Cmd =
        PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;

    for (i = 0; (i < 2); i++)
    {
        action.vlan.vlanId =
         (GT_U16)(PRV_TGF_PCL_MODIFIED_VLANID_0_CNS + i);

        pattern.ruleEgrStdNotIp.common.vid = (GT_U16)
            ((i == 0)
                ? PRV_TGF_PCL_TEST_VID0
                : PRV_TGF_PCL_TEST_VID1);

        /* AUTODOC: set 2 PCL rules 0,1 with: */
        /* AUTODOC:   format - EGRESS_STD_NOT_IP */
        /* AUTODOC:   cmd=FORWARD, egressVlanId0Cmd=TAG0_CMD_MODIFY_OUTER_TAG */
        /* AUTODOC:   pattern VIDs=5,20 */
        rc = prvTgfPclRuleSet(
            ruleFormat, prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ruleIndex + i), &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
            prvTgfDevNum, ruleFormat, prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ruleIndex + i));
    }
}


/**
* @internal prvTgfPclEgrOrgVidTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic and check results
*/
static GT_VOID prvTgfPclEgrOrgVidTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS          rc         = GT_OK;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: config Egress Policy VID to extract UP from packet Tag0 */
    rc = prvTgfPclEgressKeyFieldsVidUpModeSet(
        PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclEgressKeyFieldsVidUpModeSet: %d, %d",
        prvTgfDevNum, PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E);

    prvTgfPclEgrOrgVidTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   expected packet with TAG0 VID=6, TAG1 VID=20 */
    prvTgfPclEgrOrgVidTrafficEgressVidCheck(
        PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS, 0);

    /* AUTODOC: config Egress Policy VID to extract UP from packet Tag1 */
    rc = prvTgfPclEgressKeyFieldsVidUpModeSet(
        PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG1_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclEgressKeyFieldsVidUpModeSet: %d, %d",
        prvTgfDevNum, PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG1_E);

    prvTgfPclEgrOrgVidTrafficGenerate();

    /* AUTODOC: verify traffic on port 2: */
    /* AUTODOC:   expected packet with TAG0 VID=7, TAG1 VID=20 */
    prvTgfPclEgrOrgVidTrafficEgressVidCheck(
        PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS, 1);

    /* AUTODOC: config Egress Policy VID to extract UP from packet Tag0 */
    rc = prvTgfPclEgressKeyFieldsVidUpModeSet(
        PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclEgressKeyFieldsVidUpModeSet: %d, %d",
        prvTgfDevNum, PRV_TGF_PCL_EGRESS_KEY_VID_UP_FROM_TAG0_E);
}

/**
* @internal prvTgfPclEgrOrgVidTrafficTest function
* @endinternal
*
* @brief   Full Egress Original VID field test
*/
GT_VOID prvTgfPclEgrOrgVidTrafficTest
(
    GT_VOID
)
{
    prvTgfPclEgrOrgVidVlanConfigurationSet();

    prvTgfPclEgrOrgVidPclConfigurationSet();

    prvTgfPclEgrOrgVidTrafficGenerateAndCheck();

    prvTgfPclEgrOrgVidConfigurationRestore();
}

/* EGRESS_EXT_IPV4_RACL_VACL SIP + DIP test */

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDipVlanConfigure function
* @endinternal
*
* @brief   Set test VLAN configuration
*
* @param[out] vid                      -  of ingress packet
*                                       None
*/
static GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDipVlanConfigure
(
    IN GT_U16 vid
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      ii;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG1_MAC(
        "\nVLAN %d CONFIGURATION:\n",
        PRV_TGF_PCL_TEST_VID0);
    PRV_UTF_LOG4_MAC("  Port members: [%d], [%d], [%d], [%d]\n",
            prvTgfPortsArray[0], prvTgfPortsArray[1],
            prvTgfPortsArray[2], prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 5 with all ports with TAG1 cmd */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        vid,
        PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", prvTgfDevNum);

    if(needEpclWaOnDrop == GT_TRUE)
    {
        /* the multiple egress ports cause the GM to swap descriptors between
           egress ports !!! so the DROP may happen on wrong packet that egress
           wrong port !!!! */
        /* so to avoid swaps ... make sure only single port is in the */
        PRV_UTF_LOG0_MAC(
            "due to GM bug keep only 2 ports in vlan (sender , receiver)\n");
        for(ii = 0 ; ii < prvTgfPortsNum ; ii++)
        {
            if(ii == PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS ||
               ii == PRV_TGF_PCL_SEND_PORT_INDEX_CNS)
            {
                /* leave those ports in the vlan */
                continue;
            }

            prvTgfBrgVlanMemberRemove(prvTgfDevNum,PRV_TGF_PCL_TEST_VID0,
                prvTgfPortsArray[ii]);
        }

    }



}

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDip_PclConfigure function
* @endinternal
*
* @brief   Set PCL config
*
* @param[in] egressPortNum            - egress port number
* @param[in] ruleIndex                - relatine ndex of rule
* @param[in] sipPtr                   - 4-byte array of SIP to be used in EPCL rule
* @param[in] dipPtr                   - 4-byte array of DIP to be used in EPCL rule
* @param[in] doDrop                   - indication to drop the packet
*                                       None
*/
static GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDip_PclConfigure
(
    IN  GT_U32   egressPortNum,
    IN  GT_U32   ruleIndex,
    IN  GT_U8    *sipPtr,
    IN  GT_U8    *dipPtr,
    IN  GT_BOOL   doDrop
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* AUTODOC: init PCL Engine for send port*/
    /* AUTODOC:   egress direction, lookup_0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_EXT_IPV4_RACL_VACL */
    /* AUTODOC:   ipv6Key EGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        egressPortNum,
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: dev %d, port %d", prvTgfDevNum,
        egressPortNum);

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: EPCL action - Modify VID */
    action.egressPolicy =  GT_TRUE;
    action.pktCmd       = doDrop == GT_TRUE ?
            CPSS_PACKET_CMD_DROP_HARD_E :
            CPSS_PACKET_CMD_FORWARD_E   ;

    /* AUTODOC: EPCL Mask for SIP and DIP */
    mask.ruleEgrExtIpv4RaclVacl.sip.arIP[0] = 0xFF;
    mask.ruleEgrExtIpv4RaclVacl.sip.arIP[1] = 0xFF;
    mask.ruleEgrExtIpv4RaclVacl.sip.arIP[2] = 0xFF;
    mask.ruleEgrExtIpv4RaclVacl.sip.arIP[3] = 0xFF;

    mask.ruleEgrExtIpv4RaclVacl.dip.arIP[0] = 0xFF;
    mask.ruleEgrExtIpv4RaclVacl.dip.arIP[1] = 0xFF;
    mask.ruleEgrExtIpv4RaclVacl.dip.arIP[2] = 0xFF;
    mask.ruleEgrExtIpv4RaclVacl.dip.arIP[3] = 0xFF;

    /* AUTODOC: EPCL Pattern for SIP and DIP */
    pattern.ruleEgrExtIpv4RaclVacl.sip.arIP[0] = sipPtr[0];
    pattern.ruleEgrExtIpv4RaclVacl.sip.arIP[1] = sipPtr[1];
    pattern.ruleEgrExtIpv4RaclVacl.sip.arIP[2] = sipPtr[2];
    pattern.ruleEgrExtIpv4RaclVacl.sip.arIP[3] = sipPtr[3];

    pattern.ruleEgrExtIpv4RaclVacl.dip.arIP[0] = dipPtr[0];
    pattern.ruleEgrExtIpv4RaclVacl.dip.arIP[1] = dipPtr[1];
    pattern.ruleEgrExtIpv4RaclVacl.dip.arIP[2] = dipPtr[2];
    pattern.ruleEgrExtIpv4RaclVacl.dip.arIP[3] = dipPtr[3];

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ruleIndex),
            &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d",
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E);
}

static TGF_PACKET_STC          ipv4_packetInfo;
static void buildIpv4Packet
(
    IN  GT_U8    *sipPtr,
    IN  GT_U8    *dipPtr,
    IN GT_U16 vid
)
{
    GT_U32                  ii;
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;
    TGF_PACKET_VLAN_TAG_STC *vlanTagPartPtr;
    /* AUTODOC: get default IPv4 packet */
    rc = prvTgfPacketIpv4PacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &packetInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    for(ii = 0 ; ii < 4 ; ii++)
    {
        ipv4PartPtr->srcAddr[ii] = sipPtr[ii];
        ipv4PartPtr->dstAddr[ii] = dipPtr[ii];
    }

    ipv4PartPtr->totalLen = PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS;
    ipv4PartPtr->typeOfService = 0;

    /* AUTODOC: changed Ipv4 */
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,0,ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &packetInfo, (GT_VOID *)&vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
    vlanTagPartPtr->pri = 5;
    vlanTagPartPtr->vid = vid;
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,0,vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    ipv4_packetInfo.numOfParts = packetInfo.numOfParts;
    ipv4_packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    ipv4_packetInfo.partsArray =  packetInfo.partsArray;
}

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic that should match
*/
static GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS            rc         = GT_OK;

    prvTgfEthCountersReset(prvTgfDevNum);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum,
        &ipv4_packetInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: devNum[%d]",
                                 prvTgfDevNum);

    /* send Packet from requested port  */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: devNum[%d],portNum[%d]",
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

}

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerate_nonMatched function
* @endinternal
*
* @brief   Generate traffic that should not match
*/
static GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerate_nonMatched
(
    GT_VOID
)
{
    GT_STATUS            rc         = GT_OK;
    GT_U32               numVfd = 1;
    TGF_VFD_INFO_STC     vfdArray[1];

    prvTgfEthCountersReset(prvTgfDevNum);

    /* override byte of DIP[3] */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 12/*mac SA,DA*/ + 4 /*vlan tag*/ + 2 /*0x0800*/ + 19;/* offset to last byte of DIP */
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = 0x5f - 1; /* match should be with 0x5f ... so use 0x5f-1 to get no match */

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum,
        &ipv4_packetInfo,
        1      /*burstCount*/,
        numVfd      /*numVfd*/,
        vfdArray   /*vfdArray*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: devNum[%d]",
                                 prvTgfDevNum);

    /* send Packet from requested port  */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: devNum[%d],portNum[%d]",
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

}

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic and check results
*/
static GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerateAndCheck
(
    IN GT_BOOL  doDrop
)
{
    GT_STATUS          rc         = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC  portCntrs;
    GT_U32  counter;

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerate();

    /* AUTODOC: read MAC counters on EPCL related egress  port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS]);

    if(needEpclWaOnDrop == GT_TRUE)
    {
        counter = portCntrs.goodPktsRcv.l[0];
    }
    else
    {
        counter = portCntrs.goodPktsSent.l[0];
    }

    if(doDrop)
    {
        /* AUTODOC: verify packet is NOT Egress the port */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, counter,
                                     "Expected DROP : but Packet egress the port");
    }
    else
    {
        /* AUTODOC: verify packet is Egress the port */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, counter,
                                     "Expected PASS : but Packet NOT egress the port");
    }


    prvTgfEthCountersReset(prvTgfDevNum);
}

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerateAndCheck_nonMatched function
* @endinternal
*
* @brief   Generate traffic that should not match and check results
*/
static GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerateAndCheck_nonMatched
(
    void
)
{
    GT_STATUS          rc         = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC  portCntrs;
    GT_U32  counter;

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerate_nonMatched();

    /* AUTODOC: read MAC counters on EPCL related egress  port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS]);

    if(needEpclWaOnDrop == GT_TRUE)
    {
        counter = portCntrs.goodPktsRcv.l[0];
    }
    else
    {
        counter = portCntrs.goodPktsSent.l[0];
    }

    /* AUTODOC: verify packet is Egress the port */
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0,counter,
         "Expected PASS : but Packet NOT egress the port");

    prvTgfEthCountersReset(prvTgfDevNum);
}

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDipTest function
* @endinternal
*
* @brief   EGRESS_EXT_IPV4_RACL_VACL SIP and DIP test
*/
GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDipTest
(
    GT_VOID
)
{
    GT_U8    sipPtr[4] = {0xa5,0xf7,0x7f,0x5a};
    GT_U8    dipPtr[4] = {0xb5,0xe7,0x7a,0x5f};
    GT_STATUS               rc;
    CPSS_INTERFACE_INFO_STC egressPortInterface;
#ifdef GM_USED
    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* THE GM hold BUG !!! that even if the EPCL drops the packet still it is
           counted at the egress MAC countes as 'goodPktsSent' !!! .

           so AS WA , I set here mac loopback on the egress port and check if the
           INGRESS of that port got packets or not .
        */
        needEpclWaOnDrop = GT_TRUE;
        PRV_UTF_LOG0_MAC(
            "\n The GM need WA for EPCL DROP actions \n");
    }
#endif /*GM_USED*/

    /* AUTODOC: build IPV4 packet with needed SIP,DIP and vlan tag */
    buildIpv4Packet(sipPtr, dipPtr,PRV_TGF_PCL_TEST_VID0);

    PRV_UTF_LOG0_MAC(
        "\n start config \n");

    /* AUTODOC: define vlan for the needed vlan tag */
    prvTgfPclEgrExtIpv4RaclVaclSipDipVlanConfigure(
        PRV_TGF_PCL_TEST_VID0);

    if(needEpclWaOnDrop == GT_TRUE)
    {
        egressPortInterface.type = CPSS_INTERFACE_PORT_E;
        egressPortInterface.devPort.hwDevNum     =
            prvTgfDevsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS];
        egressPortInterface.devPort.portNum    =
            prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS];

        /* enable capture on receive port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK , rc, "tgfTrafficGeneratorPortTxEthCaptureSet");
    }

    PRV_UTF_LOG0_MAC(
        "\n  define EPCL rule EXT ipv4 RACL/VACL to match the SIP and DIP and to drop the packet \n");
    /* AUTODOC: define EPCL rule EXT ipv4 RACL/VACL to match the SIP and DIP and to drop the packet */
    prvTgfPclEgrExtIpv4RaclVaclSipDip_PclConfigure(
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        0 /*ruleIndex*/, sipPtr, dipPtr, GT_TRUE /*drop*/);

    PRV_UTF_LOG0_MAC(
        "\n send packet and check that dropped \n");
    /* AUTODOC: send packet and check that dropped */
    prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerateAndCheck(GT_TRUE /*drop*/);

    PRV_UTF_LOG0_MAC(
        "\n send packet with different DIP[3] that should not match ..."
        "should not be dropped \n");
    /* AUTODOC: send packet with different DIP[3] that should not match ...
        should not be dropped */
    prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerateAndCheck_nonMatched();

    PRV_UTF_LOG0_MAC(
        "\n set the rule with action 'pass' \n");
    /* AUTODOC: set the rule with action 'pass' */
    prvTgfPclEgrExtIpv4RaclVaclSipDip_PclConfigure(
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        0 /*ruleIndex*/, sipPtr, dipPtr, GT_FALSE /* no drop*/);

    PRV_UTF_LOG0_MAC(
        "\n send packet and check that NOT dropped \n");
    /* AUTODOC: send packet and check that NOT dropped */
    prvTgfPclEgrExtIpv4RaclVaclSipDipTrafficGenerateAndCheck(GT_FALSE /* no drop*/);

    PRV_UTF_LOG0_MAC(
        "\n restore config \n");

    if(needEpclWaOnDrop == GT_TRUE)
    {
        /* disable capture on receive port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK , rc, "tgfTrafficGeneratorPortTxEthCaptureSet");
    }

    prvTgfPclEgrOrgVidConfigurationRestore();
}

/**
* @internal prvTgfPclEgrUltraIpv6RaclVaclSipDip_PclConfigure function
* @endinternal
*
* @brief   Set PCL config
*
* @param[in] egressPortNum            - egress port number
* @param[in] ruleIndex                - relatine ndex of rule
* @param[in] sipPtr                   - 16-byte array of SIP to be used in EPCL rule
* @param[in] dipPtr                   - 16-byte array of DIP to be used in EPCL rule
* @param[in] doDrop                   - indication to drop the packet
*                                       None
*/
static GT_VOID prvTgfPclEgrUltraIpv6RaclVaclSipDip_PclConfigure
(
    IN  GT_U32   egressPortNum,
    IN  GT_U32   ruleIndex,
    IN  GT_U8    *sipPtr,
    IN  GT_U8    *dipPtr,
    IN  GT_BOOL   doDrop
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;
    GT_U32                      ii;

    /* AUTODOC: init PCL Engine for send port*/
    /* AUTODOC:   egress direction, lookup_0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_EXT_IPV4_RACL_VACL */
    /* AUTODOC:   ipv6Key PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E */
    rc = prvTgfPclDefPortInit(
        egressPortNum,
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: dev %d, port %d", prvTgfDevNum,
        egressPortNum);

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: EPCL action - Modify VID */
    action.egressPolicy =  GT_TRUE;
    action.pktCmd       = doDrop == GT_TRUE ?
            CPSS_PACKET_CMD_DROP_HARD_E :
            CPSS_PACKET_CMD_FORWARD_E   ;

    /* AUTODOC: EPCL Mask for SIP and DIP */
    /* AUTODOC: EPCL Pattern for SIP and DIP */
    for(ii = 0 ; ii < 16 ; ii++)
    {
        mask.ruleEgrUltraIpv6RaclVacl.sip.arIP[ii] = 0xFF;
        mask.ruleEgrUltraIpv6RaclVacl.dip.arIP[ii] = 0xFF;
        pattern.ruleEgrUltraIpv6RaclVacl.sip.arIP[ii] = sipPtr[ii];
        pattern.ruleEgrUltraIpv6RaclVacl.dip.arIP[ii] = dipPtr[ii];
    }

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E,
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(ruleIndex),
            &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d",
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E);
}

static TGF_PACKET_STC          ipv6_packetInfo;
static void buildIpv6Packet
(
    IN  GT_U8    *sipPtr,
    IN  GT_U8    *dipPtr,
    IN GT_U16 vid
)
{
    GT_U32                  ii;
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV6_STC     *ipv6PartPtr;
    TGF_PACKET_VLAN_TAG_STC *vlanTagPartPtr;
    /* AUTODOC: get default IPv6 packet */
    rc = prvTgfPacketIpv6PacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv6PacketDefaultPacketGet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV6_E, &packetInfo, (GT_VOID *)&ipv6PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    /* the IPV6 addresses defined as array of 'GT_U16' */
    for(ii = 0 ; ii < 16 ; ii += 2)
    {
        ipv6PartPtr->srcAddr[ii/2] = (GT_U16)(sipPtr[ii] << 8 | sipPtr[ii+1]);
        ipv6PartPtr->dstAddr[ii/2] = (GT_U16)(dipPtr[ii] << 8 | dipPtr[ii+1]);
    }

    /* AUTODOC: changed Ipv6 */
    rc = prvTgfPacketIpv6PacketHeaderDataSet(TGF_PACKET_PART_IPV6_E,0,ipv6PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &packetInfo, (GT_VOID *)&vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
    vlanTagPartPtr->pri = 5;
    vlanTagPartPtr->vid = vid;
    rc = prvTgfPacketIpv6PacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,0,vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    ipv6_packetInfo.numOfParts = packetInfo.numOfParts;
    ipv6_packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    ipv6_packetInfo.partsArray =  packetInfo.partsArray;
}
/**
* @internal prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic that should match
*/
static GT_VOID prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS            rc         = GT_OK;

    prvTgfEthCountersReset(prvTgfDevNum);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum,
        &ipv6_packetInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: devNum[%d]",
                                 prvTgfDevNum);

    /* send Packet from requested port  */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: devNum[%d],portNum[%d]",
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

}
/**
* @internal prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic and check results
*/
static GT_VOID prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerateAndCheck
(
    IN GT_BOOL  doDrop
)
{
    GT_STATUS          rc         = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC  portCntrs;
    GT_U32  counter;

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerate();

    /* AUTODOC: read MAC counters on EPCL related egress  port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS]);

    if(needEpclWaOnDrop == GT_TRUE)
    {
        counter = portCntrs.goodPktsRcv.l[0];
    }
    else
    {
        counter = portCntrs.goodPktsSent.l[0];
    }

    if(doDrop)
    {
        /* AUTODOC: verify packet is NOT Egress the port */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, counter,
                                     "Expected DROP : but Packet egress the port");
    }
    else
    {
        /* AUTODOC: verify packet is Egress the port */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, counter,
                                     "Expected PASS : but Packet NOT egress the port");
    }


    prvTgfEthCountersReset(prvTgfDevNum);
}
/**
* @internal prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerate_nonMatched function
* @endinternal
*
* @brief   Generate traffic that should not match
*/
static GT_VOID prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerate_nonMatched
(
    GT_VOID
)
{
    GT_STATUS            rc         = GT_OK;
    GT_U32               numVfd = 1;
    TGF_VFD_INFO_STC     vfdArray[1];

    prvTgfEthCountersReset(prvTgfDevNum);

    /* override byte of DIP[5] */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 12/*mac SA,DA*/ + 4 /*vlan tag*/ + 2 /*0x86DD*/ +
                                    8/*first fields of IPv6*/ +
                                    16/*SIP bytes*/ +
                                    5;/* offset to DIP[5] */
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = 0x6e - 1; /* match should be with 0x6e ... so use 0x6e-1 to get no match */

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum,
        &ipv6_packetInfo,
        1      /*burstCount*/,
        numVfd      /*numVfd*/,
        vfdArray   /*vfdArray*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: devNum[%d]",
                                 prvTgfDevNum);

    /* send Packet from requested port  */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: devNum[%d],portNum[%d]",
        prvTgfDevsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PCL_SEND_PORT_INDEX_CNS]);

}
/**
* @internal prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerateAndCheck_nonMatched function
* @endinternal
*
* @brief   Generate traffic that should not match and check results
*/
static GT_VOID prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerateAndCheck_nonMatched
(
    void
)
{
    GT_STATUS          rc         = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC  portCntrs;
    GT_U32  counter;

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerate_nonMatched();

    /* AUTODOC: read MAC counters on EPCL related egress  port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS]);

    if(needEpclWaOnDrop == GT_TRUE)
    {
        counter = portCntrs.goodPktsRcv.l[0];
    }
    else
    {
        counter = portCntrs.goodPktsSent.l[0];
    }

    /* AUTODOC: verify packet is Egress the port */
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0,counter,
         "Expected PASS : but Packet NOT egress the port");

    prvTgfEthCountersReset(prvTgfDevNum);
}

/**
* @internal prvTgfPclEgrUltraIpv6RaclVaclSipDipTest function
* @endinternal
*
* @brief   Egress PASS/DROP test on SIP and DIP of key : Egress Ultra Ipv6 Racl/Vacl
*/
GT_VOID prvTgfPclEgrUltraIpv6RaclVaclSipDipTest
(
    GT_VOID
)
{
    GT_U8    sipPtr[16] = { 0xf5,0x5f,0x7b,0xb7,
                            0xea,0xae,0xde,0xed,
                            0x99,0xaa,0xbb,0xdc,
                            0xa5,0xf7,0x7f,0x5a};
    GT_U8    dipPtr[16] = { 0xf6,0x6f,0x6b,0xb6,
                            0xe6,0x6e,0xd6,0xe6,
                            0x96,0xa6,0x6b,0x6c,
                            0xb5,0xe7,0x7a,0x5f};
    GT_STATUS               rc;
    CPSS_INTERFACE_INFO_STC egressPortInterface;
#ifdef GM_USED
    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* THE GM hold BUG !!! that even if the EPCL drops the packet still it is
           counted at the egress MAC countes as 'goodPktsSent' !!! .

           so AS WA , I set here mac loopback on the egress port and check if the
           INGRESS of that port got packets or not .
        */
        needEpclWaOnDrop = GT_TRUE;
        PRV_UTF_LOG0_MAC(
            "\n The GM need WA for EPCL DROP actions \n");
    }
#endif /*GM_USED*/

    /* AUTODOC: build IPV6 packet with needed SIP,DIP and vlan tag */
    buildIpv6Packet(sipPtr, dipPtr,PRV_TGF_PCL_TEST_VID0);

    PRV_UTF_LOG0_MAC(
        "\n start config \n");

    /* AUTODOC: define vlan for the needed vlan tag */
    prvTgfPclEgrExtIpv4RaclVaclSipDipVlanConfigure(
        PRV_TGF_PCL_TEST_VID0);

    if(needEpclWaOnDrop == GT_TRUE)
    {
        egressPortInterface.type = CPSS_INTERFACE_PORT_E;
        egressPortInterface.devPort.hwDevNum     =
            prvTgfDevsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS];
        egressPortInterface.devPort.portNum    =
            prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS];

        /* enable capture on receive port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK , rc, "tgfTrafficGeneratorPortTxEthCaptureSet");
    }

    PRV_UTF_LOG0_MAC(
        "\n  define EPCL rule Ultra ipv6 RACL/VACL to match the SIP and DIP and to drop the packet \n");
    /* AUTODOC: define EPCL rule Ultra ipv6 RACL/VACL to match the SIP and DIP and to drop the packet */
    prvTgfPclEgrUltraIpv6RaclVaclSipDip_PclConfigure(
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        0 /*ruleIndex*/, sipPtr, dipPtr, GT_TRUE /*drop*/);

    PRV_UTF_LOG0_MAC(
        "\n send packet and check that dropped \n");
    /* AUTODOC: send packet and check that dropped */
    prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerateAndCheck(GT_TRUE /*drop*/);

    PRV_UTF_LOG0_MAC(
        "\n send packet with different DIP[3] that should not match ..."
        "should not be dropped \n");
    /* AUTODOC: send packet with different DIP[5] that should not match ...
        should not be dropped */
    prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerateAndCheck_nonMatched();

    PRV_UTF_LOG0_MAC(
        "\n set the rule with action 'pass' \n");
    /* AUTODOC: set the rule with action 'pass' */
    prvTgfPclEgrUltraIpv6RaclVaclSipDip_PclConfigure(
        prvTgfPortsArray[PRV_TGF_PCL_RECEIVE_PORT_INDEX_CNS],
        0 /*ruleIndex*/, sipPtr, dipPtr, GT_FALSE /* no drop*/);

    PRV_UTF_LOG0_MAC(
        "\n send packet and check that NOT dropped \n");
    /* AUTODOC: send packet and check that NOT dropped */
    prvTgfPclEgrUltraIpv6RaclVaclSipDipTrafficGenerateAndCheck(GT_FALSE /* no drop*/);

    PRV_UTF_LOG0_MAC(
        "\n restore config \n");

    if(needEpclWaOnDrop == GT_TRUE)
    {
        /* disable capture on receive port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK , rc, "tgfTrafficGeneratorPortTxEthCaptureSet");
    }

    prvTgfPclEgrOrgVidConfigurationRestore();
}


