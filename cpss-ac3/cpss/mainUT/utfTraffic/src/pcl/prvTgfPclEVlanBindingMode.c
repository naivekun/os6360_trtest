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
* @file prvTgfPclEVlanBindingMode.c
*
* @brief PCL eVLAN based binding mode advanced UTs.
*
* @version   3
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

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>

#include <pcl/prvTgfPclEVlanBindingMode.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_E_VLANID_1_CNS        ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(prvTgfEvlanBase + 14)
#define PRV_TGF_E_VLANID_2_CNS        ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(prvTgfEvlanBase + 15)
#define PRV_TGF_E_VLANID_3_CNS        ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(prvTgfEvlanBase + 16)

#define PRV_TGF_E_VLANID_4_CNS        7
#define PRV_TGF_E_VLANID_5_CNS        8
#define PRV_TGF_E_VLANID_6_CNS        9


/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* initialized dynamically, used to make eVlan = <base>+<number>
 * either < 4K  - for AC3X (88E1690 ports doesn't support vlan >4K)
 * or > 4K - for rest of eArch-devices. */
static GT_U16 prvTgfEvlanBase;

/******************************* Test packets **********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x19},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfExpectedPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    1, 0, 1/*set in run time*/                          /* pri, cfi, VlanId */
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
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* LENGTH of expected packet */
#define PRV_TGF_PACKET_EXPECTED_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_EXPECTED_CRC_LEN_CNS  (PRV_TGF_PACKET_EXPECTED_LEN_CNS + TGF_CRC_LEN_CNS)


/* PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};
/* Expected PACKET info */
static TGF_PACKET_STC prvTgfExpectedPacketInfo = {
    PRV_TGF_PACKET_EXPECTED_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfExpectedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfExpectedPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* sending port */
#define PRV_TGF_SEND_PORT_IDX_CNS    0

/* target port */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS 1

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_0_CNS  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0,0)
#define PRV_TGF_IPCL_RULE_INDEX_1_CNS  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(1,1)
#define PRV_TGF_IPCL_RULE_INDEX_2_CNS  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(2,2)

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_0_CNS  12
#define PRV_TGF_IPCL_PCL_ID_1_CNS  13
#define PRV_TGF_IPCL_PCL_ID_2_CNS  14

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclVlanPortVidSet function
* @endinternal
*
* @brief   Set ports to be assigned with eVLANs.
*
* @param[in] vlanId                   -  to be assigned
*                                       None
*/
static GT_VOID prvTgfPclVlanPortVidSet
(
    IN GT_U16           vlanId
)
{
    GT_STATUS rc          = GT_OK;

    rc = prvTgfBrgVlanPortVidSet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
}

/**
* @internal prvTgfPclEVlanBindingModeTestPacketSend function
* @endinternal
*
* @brief   Function sends packet and check results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclEVlanBindingModeTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
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
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

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
* @internal prvTgfPclEVlanBindingModeTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packetand and performs check for expected results.
*
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPclEVlanBindingModeTestSendAndCheck
(
    IN GT_U32           callIdentifier
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
    prvTgfPclEVlanBindingModeTestPacketSend(&portInterface, &prvTgfPacketInfo);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);
            continue;
        }

        /* check Tx counters */
        if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);

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

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     "port = %d, rc = 0x%02X, %d\n",
                                     portInterface.devPort.portNum, rc, callIdentifier);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
    }
}

/**
* @internal prvTgfPclEVlanBindingModeConfigForLookup function
* @endinternal
*
* @brief   Configure test mode for Lookup
*
* @param[in] vlanId                   - vlanId
* @param[in] lookupNum                - Lookup number
* @param[in] pclId                    - PCL-ID
* @param[in] vlanIdMod                - vlanId to be changing to
* @param[in] ruleIndex                - index of the rule in the TCAM
*                                       None
*/
static GT_VOID prvTgfPclEVlanBindingModeConfigForLookup
(
    IN GT_U16                        vlanId,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN GT_U32                        pclId,
    IN GT_U16                        vlanIdMod,
    IN GT_U32                        ruleIndex
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = pclId;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;


    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInitExt2(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum,
        &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt2: %d", prvTgfDevNum);

    /* AUTODOC: Set access mode on eVlan */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Set PCL Configuration table entry for lookup with interface eVLAN*/
    interfaceInfo.type       = CPSS_INTERFACE_VID_E;
    interfaceInfo.vlanId     = (vlanId & 0xFFF); /* eVLAN [11:0]*/

    rc = prvTgfPclCfgTblSet( &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, lookupNum, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: add FDB entry with daMac, vlanId, receive port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac, vlanIdMod,
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE);

    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* Set PCL rule*/
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: mask for PCL-ID */
    mask.ruleStdNotIp.common.pclId    = 0x3FF;

    /* AUTODOC: pattern for PCL-ID */
    pattern.ruleStdNotIp.common.pclId = (GT_U16)pclId;

    /* AUTODOC: action - tag0 vlan value change */
    action.vlan.modifyVlan =  CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E;
    action.vlan.vlanId = vlanIdMod;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 ruleIndex);
}
/**
* @internal prvTgfPclEVlanBindingModeTestConfigurationSet function
* @endinternal
*
* @brief   Ingress PCL eVLAN based binding mode test initial configurations
*/
GT_VOID prvTgfPclEVlanBindingModeTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5004 with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( PRV_TGF_E_VLANID_1_CNS,
                                                   PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                  prvTgfDevNum);

    /* AUTODOC: create VLAN 5005 with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( PRV_TGF_E_VLANID_2_CNS,
                                                   PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                  prvTgfDevNum);

    /* AUTODOC: create VLAN 5006 with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( PRV_TGF_E_VLANID_3_CNS,
                                                   PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                  prvTgfDevNum);

    /* AUTODOC: create VLAN 7 with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( PRV_TGF_E_VLANID_4_CNS,
                                                   PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                  prvTgfDevNum);

    /* AUTODOC: create VLAN 8 with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( PRV_TGF_E_VLANID_5_CNS,
                                                   PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                  prvTgfDevNum);

    /* AUTODOC: create VLAN 9 with all tagged port */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd( PRV_TGF_E_VLANID_6_CNS,
                                                   PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC( GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                  prvTgfDevNum);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Configure mode for  CPSS_PCL_LOOKUP_NUMBER_0_E*/
    prvTgfPclEVlanBindingModeConfigForLookup( PRV_TGF_E_VLANID_1_CNS,
                                  CPSS_PCL_LOOKUP_NUMBER_0_E,
                                  PRV_TGF_IPCL_PCL_ID_0_CNS,
                                  PRV_TGF_E_VLANID_4_CNS,
                                  PRV_TGF_IPCL_RULE_INDEX_0_CNS );

    /* AUTODOC: Configure mode for  CPSS_PCL_LOOKUP_NUMBER_1_E*/
    prvTgfPclEVlanBindingModeConfigForLookup( PRV_TGF_E_VLANID_2_CNS,
                                  CPSS_PCL_LOOKUP_NUMBER_1_E,
                                  PRV_TGF_IPCL_PCL_ID_1_CNS,
                                  PRV_TGF_E_VLANID_5_CNS,
                                  PRV_TGF_IPCL_RULE_INDEX_1_CNS );

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: Configure mode for  CPSS_PCL_LOOKUP_NUMBER_2_E*/
        prvTgfPclEVlanBindingModeConfigForLookup( PRV_TGF_E_VLANID_3_CNS,
                                      CPSS_PCL_LOOKUP_NUMBER_2_E,
                                      PRV_TGF_IPCL_PCL_ID_2_CNS,
                                      PRV_TGF_E_VLANID_6_CNS,
                                      PRV_TGF_IPCL_RULE_INDEX_2_CNS );
    }
}

/**
* @internal prvTgfPclEVlanBindingModeTestConfigurationRestore function
* @endinternal
*
* @brief   Ingress PCL eVLAN based binding mode test restore configurations
*/
GT_VOID prvTgfPclEVlanBindingModeTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_IPCL_RULE_INDEX_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PRV_TGF_IPCL_RULE_INDEX_0_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_IPCL_RULE_INDEX_1_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PRV_TGF_IPCL_RULE_INDEX_1_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_IPCL_RULE_INDEX_2_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PRV_TGF_IPCL_RULE_INDEX_2_CNS,
                                 GT_FALSE);

    /* AUTODOC : disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: Restore default access mode for LOOKUP 0 on ePort */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet", CPSS_PCL_LOOKUP_NUMBER_0_E);

    /* AUTODOC: Restore default access mode for LOOKUP 1 on ePort */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_1_E, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet", CPSS_PCL_LOOKUP_NUMBER_1_E);

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: Restore default access mode for LOOKUP 2 on ePort */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_2_E, 0 /*sublookup*/,
            PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet", CPSS_PCL_LOOKUP_NUMBER_2_E);
    }

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(  GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                   prvTgfDevNum, prvTgfDefVlanId);

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

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_5_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_6_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_6_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclEVlanBindingModeTest function
* @endinternal
*
* @brief   IPCL eVLAN based binding mode test
*/
GT_VOID prvTgfPclEVlanBindingModeTest
(
    GT_VOID
)
{
    GT_U32    sendCallInst = 0;

    prvTgfEvlanBase = (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum)) ?
        1000 : 5000;

    /* AUTODOC: Set test configuration */
    prvTgfPclEVlanBindingModeTestConfigurationSet();

    /* AUTODOC: assign send port with eVLAN 5004*/
    prvTgfPclVlanPortVidSet(PRV_TGF_E_VLANID_1_CNS);

    prvTgfExpectedPacketVlanTagPart.vid = PRV_TGF_E_VLANID_4_CNS;

    /* AUTODOC: send packet and check results*/
    prvTgfPclEVlanBindingModeTestSendAndCheck(sendCallInst++/*0*/);

    /* AUTODOC: assign send port with eVLAN 5005*/
    prvTgfPclVlanPortVidSet(PRV_TGF_E_VLANID_2_CNS);

    prvTgfExpectedPacketVlanTagPart.vid = PRV_TGF_E_VLANID_5_CNS;

    /* AUTODOC: send packet and check results */
    prvTgfPclEVlanBindingModeTestSendAndCheck(sendCallInst++/*1*/);

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: assign send port with eVLAN 5006*/
        prvTgfPclVlanPortVidSet(PRV_TGF_E_VLANID_3_CNS);

        prvTgfExpectedPacketVlanTagPart.vid = PRV_TGF_E_VLANID_6_CNS;
    }

    /* AUTODOC: send packet and check results */
    prvTgfPclEVlanBindingModeTestSendAndCheck(sendCallInst++/*2*/);

    prvTgfPclEVlanBindingModeTestConfigurationRestore();
}


