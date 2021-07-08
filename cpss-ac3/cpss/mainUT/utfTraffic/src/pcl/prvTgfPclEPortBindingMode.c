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
* @file prvTgfPclEPortBindingMode.c
*
* @brief PCL ePort based binding mode advanced UTs.
*
* @version   5
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
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>

#include <pcl/prvTgfPclEPortBindingMode.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default eVLAN Id */
#define PRV_TGF_E_VLANID_1_CNS        5

#define PRV_TGF_E_VLANID_2_CNS        3
#define PRV_TGF_E_VLANID_3_CNS        4095
/* make sure that PRV_TGF_E_VLANID_3_CNS and PRV_TGF_E_VLANID_4_CNS not use the same number */
#define PRV_TGF_E_VLANID_4_CNS        ((ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(8191)) > _4K) ? \
                                      (ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(8191)) :         \
                                      (PRV_TGF_E_VLANID_3_CNS - 1)

/* translated VLAN Id */
#define PRV_TGF_VLANID_2_CNS          3
#define PRV_TGF_VLANID_3_CNS          4095
#define PRV_TGF_VLANID_4_CNS          4000

/* VLAN Range value */
#define PRV_TGF_VLAN_RANGE_CNS        8191


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
    0, 0, PRV_TGF_E_VLANID_1_CNS                          /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfExpectedPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_E_VLANID_1_CNS                          /* pri, cfi, VlanId */
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
#define PRV_TGF_SEND_PORT_IDX_CNS    0

/* target port */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS 1

#define SET_ARR_3_MAC(arr,a,b,c) \
    arr[0] = a;                \
    arr[1] = b;                \
    arr[2] = c

/* Send ePort values */
#define PRV_TGF_SEND_EPORT_VALUES_MAC(arr) \
    SET_ARR_3_MAC(arr ,                               \
                ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(600),    \
                ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(5005) ,  \
                ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(8191) )

/* PCL rule indexes */
#define PRV_TGF_IPCL_RULE_INDEX_0_CNS  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0,0)
#define PRV_TGF_IPCL_RULE_INDEX_1_CNS  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(1,1)
#define PRV_TGF_IPCL_RULE_INDEX_2_CNS  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(2,2)

/* TTI rule indexes */
#define PRV_TGF_TTI_RULE_INDEX_CNS   (prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3))

/* PCL IDs */
#define PRV_TGF_IPCL_PCL_ID_0_CNS  12
#define PRV_TGF_IPCL_PCL_ID_1_CNS  13
#define PRV_TGF_IPCL_PCL_ID_2_CNS  14

/* stored default Vlan ID */
static GT_U16   prvTgfDefVlanId = 0;
static PRV_TGF_BRG_VLAN_TRANSLATION_ENT vlanPortTranslationEnableDef;
static GT_U16  vlanTranslationEntryDef[3];
static GT_U16  vidRangeDef;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclEPortBindingModeTtiSrcEportSet function
* @endinternal
*
* @brief   Configure Tunnel rule and action
*
* @param[in] srcPhyPort               - source physical port
* @param[in] sourceEPort              - source ePort
*                                       None
*/
static GT_VOID prvTgfPclEPortBindingModeTtiSrcEportSet
(
    IN GT_U32              srcPhyPort,
    IN GT_U32              sourceEPort
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC      ttiAction;
    PRV_TGF_TTI_ACTION_2_STC    ttiAction2;
    PRV_TGF_TTI_RULE_UNT        ttiPattern;
    PRV_TGF_TTI_RULE_UNT        ttiMask;

    /* AUTODOC: enable TTI lookup for Ethernet key on port 1 */
    rc = prvTgfTtiPortLookupEnableSet(
        srcPhyPort, PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* set TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    ttiMask.eth.common.srcPortTrunk    = 0xFF;
    ttiPattern.eth.common.srcPortTrunk = srcPhyPort;

    /* set TTI mask - dummy, will be overridden */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.tag0VlanCmd = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;

    /* AUTODOC: add TTI Rule 8 with: */
    /* AUTODOC:   key=TTI_KEY_ETH, srcPortTrunk=1 */
    /* AUTODOC:   tag1VlanCmd=MODIFY_UNTAGGED */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_RULE_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    cpssOsMemSet(&ttiAction2, 0, sizeof(ttiAction2));
    ttiAction2.tunnelTerminate             = GT_FALSE;
    ttiAction2.command                     = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction2.redirectCommand             = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction2.tag0VlanCmd                 = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction2.sourceEPortAssignmentEnable = GT_TRUE;
    ttiAction2.sourceEPort                 = sourceEPort;

    /* AUTODOC: add TTI Rule action for Rule 8 with: */
    /* AUTODOC:   TT=GT_FALSE, cmd=FORWARD */
    /* AUTODOC:   tag1VlanCmd=MODIFY_UNTAGGED */
    /* AUTODOC:   sourceEPort=0x7FF */
    rc = prvTgfTtiRuleAction2Set(PRV_TGF_TTI_RULE_INDEX_CNS, &ttiAction2);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiRuleAction2Set: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclEPortBindingModeTtiSrcEportReset function
* @endinternal
*
* @brief   Restore TTI rule
*
* @param[in] srcPhyPort               - source physical port
*                                       None
*/
static GT_VOID prvTgfPclEPortBindingModeTtiSrcEportReset
(
    IN GT_U32 srcPhyPort
)
{
    GT_STATUS                   rc = GT_OK;

    /* AUTODOC: invalidate TTI Rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: disable TTI lookup on port 1 */
    rc = prvTgfTtiPortLookupEnableSet(
        srcPhyPort, PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclEPortBindingModeTestPacketSend function
* @endinternal
*
* @brief   Function sends packet and check results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclEPortBindingModeTestPacketSend
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
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                         TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                            portInterfacePtr->devPort.hwDevNum,
                            portInterfacePtr->devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                        TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                            portInterfacePtr->devPort.hwDevNum,
                            portInterfacePtr->devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPclEPortBindingModeTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packetand and performs check for expected results.
*
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPclEPortBindingModeTestSendAndCheck
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
    prvTgfPclEPortBindingModeTestPacketSend(&portInterface, &prvTgfPacketInfo);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
                                       GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount,
                                         portCntrs.goodPktsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);
            continue;
        }

        /* check Tx counters */
        if (PRV_TGF_RECEIVE_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount,
                                         portCntrs.goodPktsSent.l[0],
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
                                     portInterface.devPort.portNum, rc,
                                     callIdentifier);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                 "get another goodPktsSent counter than expectedon port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
    }
}

/**
* @internal prvTgfPclEPortBindingModeConfigForLookup function
* @endinternal
*
* @brief   Configure test mode for Lookup
*
* @param[in] sourceEPort              - source ePort
* @param[in] lookupNum                - Lookup number
* @param[in] pclId                    - PCL-ID
* @param[in] vlanId                   -  to be changing to
* @param[in] ruleIndex                - index of the rule in the TCAM
*                                       None
*/
static GT_VOID prvTgfPclEPortBindingModeConfigForLookup
(
    IN GT_U32                        sourceEPort,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT    lookupNum,
    IN GT_U32                        pclId,
    IN GT_U16                        vlanId,
    IN GT_U32                        ruleIndex
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
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

    /* AUTODOC: Enables ingress policy on ePort */
    rc = prvTgfPclEPortIngressPolicyEnable(
            prvTgfDevNum, sourceEPort, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclEPortIngressPolicyEnable: %d, %d, %d",
        prvTgfDevNum, sourceEPort, GT_TRUE);

    /* AUTODOC:  Enable ePort access mode and set PCL configuration entry */
    rc = prvTgfPclEportAccessModeInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        sourceEPort & 0xFFF, /* ePort (12 LSbits) */
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum,
        &lookupCfg);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfPclEportAccessModeInit: %d, %d, %d", prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], sourceEPort & 0xFFF);


    /* AUTODOC: add FDB entry with daMac, vlanId, receive port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac, vlanId,
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
    action.vlan.modifyVlan =  CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E;
    action.vlan.vlanId = vlanId;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d",
                                 ruleIndex);
}
/**
* @internal prvTgfPclEPortBindingModeTestConfigurationSet function
* @endinternal
*
* @brief   Ingress PCL ePort based binding mode test initial configurations
*/
GT_VOID prvTgfPclEPortBindingModeTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS     rc;
    GT_PORT_NUM   eSendPortArr[3];
    GT_U32         portsArray[2];
    GT_U8         tagArray[] = {1, 1};

    portsArray[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    portsArray[1] = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_TGF_SEND_EPORT_VALUES_MAC(eSendPortArr);

    /* get default vlan range */
    rc = prvTgfBrgVlanRangeGet(&vidRangeDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeGet");

    /* AUTODOC: set VLAN range*/
    rc = prvTgfBrgVlanRangeSet(PRV_TGF_VLAN_RANGE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /* AUTODOC: create VLAN 5 with tagged ports 0, 1*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_1_CNS,
                                           portsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_1_CNS);

    /* AUTODOC: create VLAN 3 with tagged ports 0, 1*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_2_CNS,
                                           portsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_2_CNS);

    /* AUTODOC: create VLAN 2111 with tagged ports 0, 1*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_3_CNS,
                                           portsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_3_CNS);

    /* AUTODOC: create VLAN 4095 with tagged ports 0, 1*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_E_VLANID_4_CNS,
                                           portsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet",
                                 PRV_TGF_E_VLANID_4_CNS);

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

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
    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_E_VLANID_2_CNS,
                                          CPSS_DIRECTION_EGRESS_E,
                                          &vlanTranslationEntryDef[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead");

    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_E_VLANID_3_CNS,
                                           CPSS_DIRECTION_EGRESS_E,
                                           &vlanTranslationEntryDef[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead");

    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, PRV_TGF_E_VLANID_4_CNS,
                                           CPSS_DIRECTION_EGRESS_E,
                                           &vlanTranslationEntryDef[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead");

    /* AUTODOC: configure translation on egress ports */
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

    /* AUTODOC: Configure mode for  CPSS_PCL_LOOKUP_NUMBER_0_E*/
    prvTgfPclEPortBindingModeConfigForLookup( eSendPortArr[0],
                                  CPSS_PCL_LOOKUP_NUMBER_0_E,
                                  PRV_TGF_IPCL_PCL_ID_0_CNS,
                                  PRV_TGF_E_VLANID_2_CNS,
                                  PRV_TGF_IPCL_RULE_INDEX_0_CNS );

    /* AUTODOC: Configure mode for  CPSS_PCL_LOOKUP_NUMBER_1_E*/
    prvTgfPclEPortBindingModeConfigForLookup( eSendPortArr[1],
                                  CPSS_PCL_LOOKUP_NUMBER_1_E,
                                  PRV_TGF_IPCL_PCL_ID_1_CNS,
                                  PRV_TGF_E_VLANID_3_CNS,
                                  PRV_TGF_IPCL_RULE_INDEX_1_CNS );

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: Configure mode for  CPSS_PCL_LOOKUP_NUMBER_2_E*/
        prvTgfPclEPortBindingModeConfigForLookup( eSendPortArr[2],
                                      CPSS_PCL_LOOKUP_NUMBER_2_E,
                                      PRV_TGF_IPCL_PCL_ID_2_CNS,
                                      PRV_TGF_E_VLANID_4_CNS,
                                      PRV_TGF_IPCL_RULE_INDEX_2_CNS );
    }
}

/**
* @internal prvTgfPclEPortBindingModeTestConfigurationRestore function
* @endinternal
*
* @brief   Ingress PCL ePort based binding mode test restore configurations
*/
GT_VOID prvTgfPclEPortBindingModeTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    GT_PORT_NUM                 eSendPortArr[3];
    CPSS_INTERFACE_INFO_STC     interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC  lookupCfg;

    /* clear PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    PRV_TGF_SEND_EPORT_VALUES_MAC(eSendPortArr);

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_IPCL_RULE_INDEX_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PRV_TGF_IPCL_RULE_INDEX_0_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_IPCL_RULE_INDEX_1_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 PRV_TGF_IPCL_RULE_INDEX_1_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     PRV_TGF_IPCL_RULE_INDEX_2_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclRuleValidStatusSet: %d, %d, %d",
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
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet",
                                 CPSS_PCL_LOOKUP_NUMBER_0_E);

    /* AUTODOC: Restore default access mode for LOOKUP 1 on ePort */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_1_E, 0 /*sublookup*/,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclPortLookupCfgTabAccessModeSet",
                                 CPSS_PCL_LOOKUP_NUMBER_1_E);

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        /* AUTODOC: Restore default access mode for LOOKUP 2 on ePort */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_2_E, 0 /*sublookup*/,
            PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfPclPortLookupCfgTabAccessModeSet",
                                     CPSS_PCL_LOOKUP_NUMBER_2_E);
    }

    interfaceInfo.type              = CPSS_INTERFACE_INDEX_E;
    interfaceInfo.index             = eSendPortArr[0] & 0xFFF; /* ePort (12 LSbits) */

    lookupCfg.enableLookup           = GT_FALSE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    /* AUTODOC: clear PCL configuration table for CPSS_PCL_LOOKUP_NUMBER_0_E */
    rc = prvTgfPclCfgTblSet( &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_NUMBER_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet",
                                 CPSS_PCL_LOOKUP_NUMBER_0_E);

    interfaceInfo.index = eSendPortArr[1] & 0xFFF; /* ePort (12 LSbits) */
    /* AUTODOC: clear PCL configuration table for CPSS_PCL_LOOKUP_NUMBER_1_E */
    rc = prvTgfPclCfgTblSet( &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_NUMBER_1_E, &lookupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet",
                                 CPSS_PCL_LOOKUP_NUMBER_1_E);

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        interfaceInfo.index = eSendPortArr[2] & 0xFFF; /* ePort (12 LSbits) */
        /* AUTODOC: clear PCL configuration table for CPSS_PCL_LOOKUP_NUMBER_2_E */
        rc = prvTgfPclCfgTblSet( &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_NUMBER_2_E, &lookupCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet",
                                     CPSS_PCL_LOOKUP_NUMBER_2_E);
    }

    /* AUTODOC: disable ingress policy on ePort 600 */
    rc = prvTgfPclEPortIngressPolicyEnable(
        prvTgfDevNum, eSendPortArr[0], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
        prvTgfDevNum, eSendPortArr[0]);

    /* AUTODOC: disable ingress policy on ePort 5005 */
    rc = prvTgfPclEPortIngressPolicyEnable(
        prvTgfDevNum, eSendPortArr[1], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
        prvTgfDevNum, eSendPortArr[1]);

    /* AUTODOC: disable ingress policy on ePort 8191 */
    rc = prvTgfPclEPortIngressPolicyEnable(
        prvTgfDevNum, eSendPortArr[2], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
        prvTgfDevNum, eSendPortArr[2]);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d",
                                 GT_FALSE);

    /* AUTODOC: Restore default ePort value */
    rc = prvTgfCfgPortDefaultSourceEportNumberSet(
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL3_STRING_MAC(
        GT_OK, rc, "prvTgfCfgPortDefaultSourceEportNumberSet: %d, %d, %d",
        prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

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
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_1_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_2_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_3_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_3_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_E_VLANID_4_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_E_VLANID_4_CNS);

   /* AUTODOC: restore default egress VLAN translation and configuration translation
    for VLAN 4 on egress port */
    rc = prvTgfBrgVlanPortTranslationEnableSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                                 CPSS_DIRECTION_EGRESS_E,
                                 vlanPortTranslationEnableDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortTranslationEnableSet");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_2_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            vlanTranslationEntryDef[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_3_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            vlanTranslationEntryDef[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum, PRV_TGF_E_VLANID_4_CNS,
                                            CPSS_DIRECTION_EGRESS_E,
                                            vlanTranslationEntryDef[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite");

    /* AUTODOC: restore default VLAN range*/
    rc = prvTgfBrgVlanRangeSet(vidRangeDef);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanRangeSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclEPortBindingModeTest function
* @endinternal
*
* @brief   IPCL ePort based binding mode test
*/
GT_VOID prvTgfPclEPortBindingModeTest
(
    GT_VOID
)
{
    GT_PORT_NUM   eSendPortArr[3];
    GT_U32        eSendPortNum = sizeof(eSendPortArr) / sizeof(eSendPortArr[0]);
    GT_U32        portIdx;
    GT_U32        vlanArr[] = {PRV_TGF_VLANID_2_CNS, PRV_TGF_VLANID_3_CNS,
                               PRV_TGF_VLANID_4_CNS};

    PRV_TGF_SEND_EPORT_VALUES_MAC(eSendPortArr);

    /* AUTODOC: Set test configuration */
    prvTgfPclEPortBindingModeTestConfigurationSet();

    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass)
    {
        eSendPortNum--;
    }
    /* AUTODOC: Iterate through eSendPortArr [600, 5005, 8191]: */
    for(portIdx = 0; portIdx < eSendPortNum; portIdx++)
    {
        PRV_UTF_LOG1_MAC( "*** Send ePort = 0x%04X  *** \n",
                                                         eSendPortArr[portIdx]);

        /* AUTODOC: Set Tunnel rule to assign src ePort*/
        prvTgfPclEPortBindingModeTtiSrcEportSet(
                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                    eSendPortArr[portIdx]);

        prvTgfExpectedPacketVlanTagPart.vid = (TGF_VLAN_ID)(vlanArr[portIdx]);

        /* AUTODOC: send packet and check results*/
        prvTgfPclEPortBindingModeTestSendAndCheck(portIdx);

        /* AUTODOC: invalidate TTI rule*/
        prvTgfPclEPortBindingModeTtiSrcEportReset(
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    prvTgfPclEPortBindingModeTestConfigurationRestore();
}


