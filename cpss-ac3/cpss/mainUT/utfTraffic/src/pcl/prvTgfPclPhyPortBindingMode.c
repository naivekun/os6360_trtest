/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPclEPortBindingMode.c
*
* DESCRIPTION:
*       PCL physical port based binding mode advanced UTs.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
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
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>

#include <pcl/prvTgfPclPhyPortBindingMode.h>
#include <pcl/prvTgfPclUdbOnlyKeys.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_E_VLANID_1_CNS     5
#define PRV_TGF_E_VLANID_2_CNS     3
#define PRV_TGF_E_VLANID_3_CNS     ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(4096+7)
#define PRV_TGF_E_VLANID_4_CNS     ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(8191)

/* translated VLAN Id */
#define PRV_TGF_VLANID_1_CNS       5
#define PRV_TGF_VLANID_2_CNS       3
#define PRV_TGF_VLANID_3_CNS       4095
#define PRV_TGF_VLANID_4_CNS       4000

/* VLAN Range value */
#define PRV_TGF_VLAN_RANGE_CNS     8191

#define PRV_TGF_UP_DEFAULT_CNS     0
#define PRV_TGF_UP_MODIFY_1_CNS    1
#define PRV_TGF_UP_MODIFY_2_CNS    4
#define PRV_TGF_UP_MODIFY_3_CNS    7

#define QOS_PROFILE_INDEX_CNS      3

#define PRV_TGF_FLOW_ID_CNS        6

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packets **********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x19},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    PRV_TGF_UP_DEFAULT_CNS, 0, PRV_TGF_E_VLANID_1_CNS   /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfExpectedPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    PRV_TGF_UP_DEFAULT_CNS, 0, PRV_TGF_E_VLANID_1_CNS   /* pri, cfi, VlanId */
};

/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of expected packet */
static TGF_PACKET_PART_STC prvTgfExpectedPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfExpectedPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)


/* PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts*/
    prvTgfPacketPartArray                                        /* partsArray*/
};
/* Expected PACKET info */
static TGF_PACKET_STC prvTgfExpectedPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfExpectedPacketPartArray) /
                                   sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfExpectedPacketPartArray                              /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* sending port */
#define PRV_TGF_SEND_1_PORT_IDX_CNS    1
#define PRV_TGF_SEND_2_PORT_IDX_CNS    2

/* target port */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS 3

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_0_CNS  0
#define PRV_TGF_IPCL_RULE_INDEX_1_CNS  1

static GT_U32   firstRuleIndex  = 0;
static GT_U32   secondRuleIndex = 0;

static void setRuleIndex(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       firstLookupNum,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       secondLookupNum
)
{
    GT_U32  lookupId;

    lookupId = firstLookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
               firstLookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
               firstLookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                         0 ;/*CPSS_PCL_LOOKUP_0_E*/

    firstRuleIndex =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,PRV_TGF_IPCL_RULE_INDEX_0_CNS);

    lookupId = secondLookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
               secondLookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
               secondLookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                         0 ;/*CPSS_PCL_LOOKUP_0_E*/

    secondRuleIndex =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,PRV_TGF_IPCL_RULE_INDEX_1_CNS);

}


/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_0_CNS  12
#define PRV_TGF_IPCL_PCL_ID_1_CNS  13

/* saved values for restore */
static PRV_TGF_COS_PROFILE_STC qosProfileSave;
static PRV_TGF_BRG_VLAN_TRANSLATION_ENT vlanPortTranslationEnableDef;
static GT_U16  vlanTranslationEntryDef[4];
static GT_U16  vidRangeDef;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclPhyPortBindingModeTestPacketSend function
* @endinternal
*
* @brief   Function sends packet and check results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] sendPort                 - send port
*                                       None
*/
static GT_VOID prvTgfPclPhyPortBindingModeTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr,
    IN GT_U32                   sendPort
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPort]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[sendPort]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPclPhyPortBindingModeTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packetand and performs check for expected results.
*
* @param[in] sendPort                 - send port
*                                       None
*/
static GT_VOID prvTgfPclPhyPortBindingModeTestSendAndCheck
(
    IN GT_U32     sendPort
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    GT_U32  actualCapturedNumOfPackets;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: send packet. */
    prvTgfPclPhyPortBindingModeTestPacketSend(&portInterface, &prvTgfPacketInfo, sendPort);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (sendPort == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);

            /* print captured packets and check TriggerCounters */
            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                    &portInterface,
                    &prvTgfExpectedPacketInfo,
                    prvTgfBurstCount,/*numOfPackets*/
                    0/*vfdNum*/,
                    NULL /*vfdArray*/,
                    NULL, /* bytesNum's skip list */
                    0,    /* length of skip list */
                    &actualCapturedNumOfPackets,
                    NULL/*onFirstPacketNumTriggersBmpPtr*/);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     "port = %d, rc = 0x%02X\n",
                                     portInterface.devPort.portNum, rc);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expectedon port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }
}

/**
* @internal prvTgfPclPhyPortBindingModeConfigForLookups function
* @endinternal
*
* @brief   Configure test mode for Lookup
*
* @param[in] firstlookupNum           - first Lookup number
* @param[in] secondlookupNum          - second Lookup number
* @param[in] eVlan                    - vlanId
* @param[in] upModify                 - UP modified
*                                       None
*/
static GT_VOID prvTgfPclPhyPortBindingModeConfigForLookups
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT    firstlookupNum,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT    secondlookupNum,
    IN GT_U16                        eVlan,
    IN GT_U32                        upModify
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    PRV_TGF_COS_PROFILE_STC          qosProfile;

    setRuleIndex(firstlookupNum,secondlookupNum);

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    /* AUTODOC: Configure first Lookup */

    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = PRV_TGF_IPCL_PCL_ID_0_CNS;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;


    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_SEND_1_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        firstlookupNum,
        &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d", prvTgfDevNum);

    /* Set PCL rule*/
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: mask for PCL-ID */
    mask.ruleIngrUdbOnly.replacedFld.pclId = 0x3FF;

    /* AUTODOC: pattern for PCL-ID */
    pattern.ruleIngrUdbOnly.replacedFld.pclId = PRV_TGF_IPCL_PCL_ID_0_CNS;


    /* AUTODOC: action - tag0 vlan value change */
    action.vlan.modifyVlan =  CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
    action.vlan.vlanId = eVlan;

    /* AUTODOC: action - flowID value change */
    action.flowId = PRV_TGF_FLOW_ID_CNS;

    rc = prvTgfPclRuleWithOptionsSet( PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                       firstRuleIndex, 0, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 firstRuleIndex);

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        firstlookupNum,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
        0xFFF /*udbReplaceBitmap*/,
        20 /*udbAmount*/,
        20 /*udbIndexBase*/,
        1  /*udbIndexIncrement*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet");

    /* AUTODOC: add FDB entry with daMac, vlanId, receive port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac, eVlan,
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: Configure second Lookup */

    lookupCfg.pclId = PRV_TGF_IPCL_PCL_ID_1_CNS;

    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_SEND_1_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        secondlookupNum,
        &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d", prvTgfDevNum);

    /* AUTODOC: Configure eVLAN binding mode */

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_1_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        secondlookupNum, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");


    /* AUTODOC: Set PCL Configuration table entry for lookup with interface eVLAN */
    interfaceInfo.type       = CPSS_INTERFACE_VID_E;
    interfaceInfo.vlanId     = (eVlan & 0xFFF); /* eVLAN [11:0]*/

    rc = prvTgfPclCfgTblSet( &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                               secondlookupNum, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* Set PCL rule*/
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));

    /* AUTODOC: mask for PCL-ID */
    mask.ruleIngrUdbOnly.replacedFld.pclId = 0x3FF;

    /* AUTODOC: pattern for PCL-ID */
    pattern.ruleIngrUdbOnly.replacedFld.pclId = PRV_TGF_IPCL_PCL_ID_1_CNS;

    /* AUTODOC: mask for flowID */
    mask.ruleIngrUdbOnly.replacedFld.flowId = 0xFFF;

    /* AUTODOC: pattern for flowID*/
    pattern.ruleIngrUdbOnly.replacedFld.flowId = PRV_TGF_FLOW_ID_CNS;

    /* AUTODOC: Save QoS profile for restore */
    rc = prvTgfCosProfileEntryGet(prvTgfDevNum, QOS_PROFILE_INDEX_CNS, &qosProfileSave);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet: %d, %d",
                                 prvTgfDevNum, QOS_PROFILE_INDEX_CNS);

    qosProfile.userPriority = upModify;

    /* AUTODOC: Configure QoS profile for action */
    rc = prvTgfCosProfileEntrySet(QOS_PROFILE_INDEX_CNS, &qosProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d",
                                 QOS_PROFILE_INDEX_CNS);

    /* AUTODOC: action - tag0 UP value change */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.qos.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    action.qos.profileIndex = QOS_PROFILE_INDEX_CNS;
    action.qos.profileAssignIndex = GT_TRUE;

    rc = prvTgfPclRuleWithOptionsSet( PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                       secondRuleIndex, 0, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 secondRuleIndex);

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        secondlookupNum,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
        0xFFF /*udbReplaceBitmap*/,
        20 /*udbAmount*/,
        20 /*udbIndexBase*/,
        1  /*udbIndexIncrement*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet");


}

/**
* @internal prvTgfPclPhyPortBindingModeRestoreConfigForLookups function
* @endinternal
*
* @brief   Restore PCL rule and binding mode for Lookup
*
* @param[in] firstlookupNum           - first Lookup number
* @param[in] secondlookupNum          - second Lookup number
*                                       None
*/
static GT_VOID prvTgfPclPhyPortBindingModeRestoreConfigForLookups
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT    firstlookupNum,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT    secondlookupNum
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC : disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     firstRuleIndex,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 firstRuleIndex,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     secondRuleIndex,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 secondRuleIndex,
                                 GT_FALSE);

    /* AUTODOC: Restore default access mode for first LOOKUP on ePort */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_1_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        firstlookupNum, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet",
                                 firstlookupNum);

    /* AUTODOC: Restore default access mode for second LOOKUP on ePort */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_1_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        secondlookupNum, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet",
                                 secondlookupNum);

    /* AUTODOC: Restore QoS profile */
    rc = prvTgfCosProfileEntrySet(QOS_PROFILE_INDEX_CNS, &qosProfileSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet: %d",
                                 QOS_PROFILE_INDEX_CNS);
}

/**
* @internal prvTgfPclPhyPortBindingModeTestConfigurationSet function
* @endinternal
*
* @brief   Ingress PCL physical port based binding mode test initial configurations
*/
GT_VOID prvTgfPclPhyPortBindingModeTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS     rc;
    GT_U32         portsArray[3];
    GT_U8         tagArray[] = {1, 1, 1};

    portsArray[0] = prvTgfPortsArray[PRV_TGF_SEND_1_PORT_IDX_CNS];
    portsArray[1] = prvTgfPortsArray[PRV_TGF_SEND_2_PORT_IDX_CNS];
    portsArray[2] = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: SETUP CONFIGURATION: */

    /* get default vlan range */
    rc = prvTgfBrgVlanRangeGet(&vidRangeDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeGet");

    /* AUTODOC: set VLAN range*/
    rc = prvTgfBrgVlanRangeSet(PRV_TGF_VLAN_RANGE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /* AUTODOC: create VLAN 5 with tagged ports 1, 2, 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_1_CNS,
                                           portsArray, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_1_CNS);

    /* AUTODOC: create VLAN 6143 with tagged ports 1, 2, 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_2_CNS,
                                           portsArray, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_2_CNS);

    /* AUTODOC: create VLAN 4096 with tagged ports 1, 2, 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_3_CNS,
                                           portsArray, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_3_CNS);

    /* AUTODOC: create VLAN 2047 with tagged ports 1, 2, 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_4_CNS,
                                           portsArray, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_4_CNS);

    /* AUTODOC: add FDB entry with daMac, vlanId, receive port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac, PRV_TGF_E_VLANID_1_CNS,
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: save default value for restore */
    rc = prvTgfBrgVlanPortTranslationEnableGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                                 CPSS_DIRECTION_EGRESS_E,
                                 &vlanPortTranslationEnableDef);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc,
                                       "prvTgfBrgVlanPortTranslationEnableGet");

    /* AUTODOC: enable egress VLAN translation */
    rc = prvTgfBrgVlanPortTranslationEnableSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                                 CPSS_DIRECTION_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc,
                                       "prvTgfBrgVlanPortTranslationEnableSet");

    /* AUTODOC: save default values for restore */
    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_E_VLANID_1_CNS,
                                          CPSS_DIRECTION_EGRESS_E,
                                          &vlanTranslationEntryDef[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead");

    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_E_VLANID_2_CNS,
                                           CPSS_DIRECTION_EGRESS_E,
                                           &vlanTranslationEntryDef[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead");

    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_E_VLANID_3_CNS,
                                           CPSS_DIRECTION_EGRESS_E,
                                           &vlanTranslationEntryDef[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead");

    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_E_VLANID_4_CNS,
                                           CPSS_DIRECTION_EGRESS_E,
                                           &vlanTranslationEntryDef[3]);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead");

    /* AUTODOC: configure translation on egress ports */
    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_1_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            PRV_TGF_VLANID_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_2_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_3_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            PRV_TGF_VLANID_3_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_4_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            PRV_TGF_VLANID_4_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");
}

/**
* @internal prvTgfPclPhyPortBindingModeTestConfigurationRestore function
* @endinternal
*
* @brief   Ingress PCL physical port based binding mode test restore configurations
*/
GT_VOID prvTgfPclPhyPortBindingModeTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_1_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_2_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_3_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_3_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_4_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_4_CNS);

   /* AUTODOC: restore default egress VLAN translation and configuration translation
    for VLAN 4 on egress port */
    rc = prvTgfBrgVlanPortTranslationEnableSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                                 CPSS_DIRECTION_EGRESS_E,
                                 vlanPortTranslationEnableDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortTranslationEnableSet");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_1_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            vlanTranslationEntryDef[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_2_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            vlanTranslationEntryDef[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_3_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            vlanTranslationEntryDef[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_4_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            vlanTranslationEntryDef[3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    /* AUTODOC: restore default VLAN range*/
    rc = prvTgfBrgVlanRangeSet(vidRangeDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclPhyPortBindingModeTest function
* @endinternal
*
* @brief   IPCL physical port based binding mode test
*/
GT_VOID prvTgfPclPhyPortBindingModeTest
(
    GT_VOID
)
{
    CPSS_PCL_LOOKUP_NUMBER_ENT    firstLookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
    CPSS_PCL_LOOKUP_NUMBER_ENT    secondLookupNum = CPSS_PCL_LOOKUP_NUMBER_1_E;

    /* AUTODOC: Set test configuration */
    prvTgfPclPhyPortBindingModeTestConfigurationSet();

    /* AUTODOC: send unmatched from port 2 packet and check results*/
    prvTgfPclPhyPortBindingModeTestSendAndCheck(PRV_TGF_SEND_2_PORT_IDX_CNS);

    /* AUTODOC: Configure mode for  IPCL0 and IPCL2 Lookups*/
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        firstLookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
        secondLookupNum = CPSS_PCL_LOOKUP_NUMBER_2_E;
    }
    prvTgfPclPhyPortBindingModeConfigForLookups( firstLookupNum,
                                                 secondLookupNum,
                                                 PRV_TGF_E_VLANID_2_CNS,
                                                 PRV_TGF_UP_MODIFY_1_CNS);

    prvTgfExpectedPacketVlanTagPart.pri = PRV_TGF_UP_MODIFY_1_CNS;
    prvTgfExpectedPacketVlanTagPart.vid = PRV_TGF_VLANID_2_CNS;

    /* AUTODOC: send matched packet and check results*/
    prvTgfPclPhyPortBindingModeTestSendAndCheck(PRV_TGF_SEND_1_PORT_IDX_CNS);

    /* AUTODOC: Configure mode for  IPCL0 and IPCL2 Lookups*/
    prvTgfPclPhyPortBindingModeRestoreConfigForLookups(
        firstLookupNum,
        secondLookupNum);

    /* AUTODOC: Configure mode for  IPCL0 and IPCL1 Lookups*/
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        firstLookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
        secondLookupNum = CPSS_PCL_LOOKUP_NUMBER_1_E;
    }
    prvTgfPclPhyPortBindingModeConfigForLookups(firstLookupNum,
                                                secondLookupNum,
                                                PRV_TGF_E_VLANID_3_CNS,
                                                PRV_TGF_UP_MODIFY_2_CNS);

    prvTgfExpectedPacketVlanTagPart.pri = PRV_TGF_UP_MODIFY_2_CNS;
    prvTgfExpectedPacketVlanTagPart.vid = PRV_TGF_VLANID_3_CNS;

    /* AUTODOC: send matched packet and check results*/
    prvTgfPclPhyPortBindingModeTestSendAndCheck(PRV_TGF_SEND_1_PORT_IDX_CNS);

    /* AUTODOC: Configure mode for  IPCL0 and IPCL1 Lookups*/
    prvTgfPclPhyPortBindingModeRestoreConfigForLookups(
        firstLookupNum,
        secondLookupNum);

    /* AUTODOC: Configure mode for  IPCL1 and IPCL2 Lookups*/
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        firstLookupNum = CPSS_PCL_LOOKUP_NUMBER_1_E;
        secondLookupNum = CPSS_PCL_LOOKUP_NUMBER_2_E;
    }
    prvTgfPclPhyPortBindingModeConfigForLookups(firstLookupNum,
                                                secondLookupNum,
                                                PRV_TGF_E_VLANID_4_CNS,
                                                PRV_TGF_UP_MODIFY_3_CNS);

    prvTgfExpectedPacketVlanTagPart.pri = PRV_TGF_UP_MODIFY_3_CNS;
    prvTgfExpectedPacketVlanTagPart.vid = PRV_TGF_VLANID_4_CNS;

    /* AUTODOC: send matched packet and check results*/
    prvTgfPclPhyPortBindingModeTestSendAndCheck(PRV_TGF_SEND_1_PORT_IDX_CNS);

    /* AUTODOC: Configure mode for  IPCL1 and IPCL2 Lookups*/
    prvTgfPclPhyPortBindingModeRestoreConfigForLookups(
        firstLookupNum,
        secondLookupNum);


    prvTgfExpectedPacketVlanTagPart.pri = PRV_TGF_UP_DEFAULT_CNS;
    prvTgfExpectedPacketVlanTagPart.vid = PRV_TGF_E_VLANID_1_CNS;

    prvTgfPclPhyPortBindingModeTestConfigurationRestore();
}

