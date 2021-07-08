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
* @file tgfCosEgressQoSdscpUpRemapping.c
*
* @brief Egress Interface Based QoS Remarking.
*
* @version   11
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
#include <common/tgfIpGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>
#include <common/tgfCosGen.h>
#include <cos/tgfCosEgressQoSdscpUpRemapping.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5

/* Traffic Classes used in test */
#define PRV_TGF_COS_TEST_TC0 3

/* vlan tag type */
CPSS_VLAN_TAG_TYPE_ENT vlanTagTypeGet;

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x54

/* Qos Profiles used in test */
static GT_U32 qosProfileIndex = 1001;

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;
static CPSS_QOS_PORT_TRUST_MODE_ENT modeGet;
static PRV_TGF_COS_PROFILE_STC        profileEntryGet;
static GT_BOOL useUpAsIndexGet;
static GT_U32 mappingTableIndexGet;
static GT_U32 egressMappingTableIndexGet[3];
static GT_U32 profileIndex2UpGet;
static CPSS_QOS_ENTRY_STC  portQoSAttrCfgGet;
static GT_BOOL egressQoSUpEnable;
static GT_BOOL egressQoStcDpEnable[3];
static GT_BOOL egressQoSDscpEnable;
static GT_U32  tcDp2Up;
static GT_U32  tcDp2Exp;
static GT_U32  tcDp2Dscp;
static GT_U32  up2UpGet;
static GT_U32 dscp2DscpGet;


/*  IPv4 packet to send */
TGF_PACKET_STC prvTgfIpv4PacketInfo;

/*  IPv4 modified packet to compare */
TGF_PACKET_STC prvTgfIpv4ModifiedUpPacketInfo;
TGF_PACKET_STC prvTgfIpv4ModifiedDscpPacketInfo;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfCosEgressQoSdscpUpRemappingBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingBridgeConfigSet
(
     GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0, 0, 0, 0};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* add tags */
    tagArray[0] = (GT_U8)GT_TRUE;
    tagArray[1] = (GT_U8)GT_TRUE;
    tagArray[2] = (GT_U8)GT_TRUE;
    tagArray[3] = (GT_U8)GT_TRUE;
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged\tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_5_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfCosEgressQoSdscpUpRemappingEgrRemarkingConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] testNumber               -  test number
*                                       None
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingEgrRemarkingConfigurationSet
(
    IN GT_U32 testNumber
)
{
    GT_STATUS                   rc = GT_OK;
    PRV_TGF_COS_PROFILE_STC        profileEntry = {0,0,0,0,0};
    CPSS_QOS_PORT_TRUST_MODE_ENT  mode;
    GT_U8    up = 5; /* incoming up */
    GT_U32 mappingTableIndex = 1;
    GT_U8    cfiDeiBit = 0;
    CPSS_QOS_ENTRY_STC      portQoSAttrCfg;
    GT_U32 tc = 3;
    CPSS_DP_LEVEL_ENT  dp = CPSS_DP_GREEN_E;
    GT_U32 i = 0;
    mode = CPSS_QOS_PORT_TRUST_L2_E;
    /* save current trust mode */
    rc = prvTgfCosPortQosTrustModeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &modeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortQosTrustModeGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    /* set new CPSS_QOS_PORT_TRUST_L2_E trust mode */
    rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], mode);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], mode);

    /* save current vlan tag type to trust*/
    rc = prvTgfCosL2TrustModeVlanTagSelectGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE,&vlanTagTypeGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosL2TrustModeVlanTagSelectGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set vlan tag1 to trust*/
    rc = prvTgfCosL2TrustModeVlanTagSelectSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE,CPSS_VLAN_TAG1_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
               "prvTgfCosL2TrustModeVlanTagSelectSet: %d, %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_VLAN_TAG1_E);

    /* get default eport trust mapping table select */
     rc = prvTgfCosPortTrustQosMappingTableIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],&useUpAsIndexGet,&mappingTableIndexGet);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,  "prvTgfCosPortTrustQosMappingTableIndexGet: %d, %d",prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
      /* set default eport trust mapping table select 1 */

     rc = prvTgfCosPortTrustQosMappingTableIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_FALSE,mappingTableIndex);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortTrustQosMappingTableIndexSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
     /* get mapped to up QoS profile in mapping table 1 */
     rc = prvTgfCosUpCfiDeiToProfileMapGet(prvTgfDevNum, mappingTableIndex,0, up, cfiDeiBit, &profileIndex2UpGet);
     UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                "prvTgfCosUpCfiDeiToProfileMapGet: %d, %d, %d , %d", prvTgfDevNum, profileIndex2UpGet, up, cfiDeiBit);

     rc = prvTgfCosUpCfiDeiToProfileMapSet(mappingTableIndex,0, up, cfiDeiBit, qosProfileIndex);
     UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
               "prvTgfCosUpCfiDeiToProfileMapSet: %d, %d, %d, %d", prvTgfDevNum,up, cfiDeiBit,qosProfileIndex);

     /* get QoS profile 1 entry */
     rc = prvTgfCosProfileEntryGet(prvTgfDevNum,qosProfileIndex,&profileEntryGet);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntryGet");

     /* set QoS profile 1 with TC3 */
     profileEntry.dropPrecedence = CPSS_DP_GREEN_E;
     profileEntry.dscp = 0;
     profileEntry.exp = 2;
     profileEntry.trafficClass = 3;
     profileEntry.userPriority = 5;
    rc = prvTgfCosProfileEntrySet(qosProfileIndex, &profileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");
    /* get QoS port attributes */
    rc = prvTgfCosPortQosConfigGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQoSAttrCfgGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortQosConfigGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    /* set QoS port attributes */
    portQoSAttrCfg.qosProfileId     = qosProfileIndex;
    portQoSAttrCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    portQoSAttrCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    portQoSAttrCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQoSAttrCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortQosConfigSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* egress QoS configuration */
    if (testNumber == 1)
    {
        for (i = 1; i < 3; i++)
        {
            /* get egress QoS TC+DP enable state */
            rc = prvTgfCosPortEgressQoStcDpMappingEnableGet(prvTgfDevNum, prvTgfPortsArray[i], &egressQoStcDpEnable[i]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                       "prvTgfCosPortEgressQoStcDpMappingEnableGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[i]);

           /* set egress QoS TC+DP enable state */
            rc = prvTgfChCosPortEgressQoStcDpMappingEnableSet(prvTgfDevNum, prvTgfPortsArray[i], GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                      "prvTgfCosPortEgressQoSUpMappingEnableSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[i]);
        }
    }

    /* get egress QoS up enable state */
    rc = prvTgfCosPortEgressQoSUpMappingEnableGet(prvTgfDevNum, prvTgfPortsArray[1], &egressQoSUpEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortEgressQoSTcDpMappingEnableGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[1]);

    /* set egress QoS up enable state */
    rc = prvTgfCosPortEgressQoSUpMappingEnableSet(prvTgfDevNum, prvTgfPortsArray[1], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortEgressQoSUpMappingEnableSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[1]);

    /* get egress QoS dscp enable state */
    rc = prvTgfCosPortEgressQoSDscpMappingEnableGet(prvTgfDevNum, prvTgfPortsArray[2], &egressQoSDscpEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortEgressQoSTcDpMappingEnableGet: %d, %d", prvTgfDevNum, prvTgfPortsArray[2]);

    /* set egress QoS dscp enable state */
    rc = prvTgfCosPortEgressQoSDscpMappingEnableSet(prvTgfDevNum, prvTgfPortsArray[2], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortEgressQoSUpMappingEnableSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[2]);

    for (i = 1; i < 3; i++)
    {
    /* get egress eport  mapping table select */
     rc = prvTgfCosPortEgressQoSMappingTableIndexGet(prvTgfDevNum, prvTgfPortsArray[i],&egressMappingTableIndexGet[i]);
     UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfDevNum, prvTgfPortsArray[i]);
     /* set egress eport  mapping table select */
     rc = prvTgfCosPortEgressQoSMappingTableIndexSet(prvTgfDevNum, prvTgfPortsArray[i],1);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortEgressQoSMappingTableIndexSet: %d, %d", prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    }
    if (testNumber == 1)
    {
    /* get egress eport  mapping table select */
     rc = prvTgfCosEgressTcDp2UpExpDscpMappingEntryGet(prvTgfDevNum, 1, tc,dp,&tcDp2Up,&tcDp2Exp,&tcDp2Dscp);
     UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    /* set egress eport  mapping table select */
     rc = prvTgfCosEgressTcDp2UpExpDscpMappingEntrySet(prvTgfDevNum, 1, tc,dp,4,0,10);
     UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    }
    if (testNumber == 2)
    {
        /* get egress eport up->up mapping */
         rc = prvTgfCosEgressUp2UpMappingEntryGet(prvTgfDevNum, 1, up, &up2UpGet);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
        /* set egress eport  up->up mapping */
         rc = prvTgfCosEgressUp2UpMappingEntrySet(prvTgfDevNum, 1, up, 6);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
        /* get egress eport dscp->dscp mapping */
         rc = prvTgfCosEgressDscp2DscpMappingEntryGet(prvTgfDevNum, 1, 0, &dscp2DscpGet);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
         /* set egress eport dscp->dscp mapping */
         rc = prvTgfCosEgressDscp2DscpMappingEntrySet(prvTgfDevNum, 1, 0, 61);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    }
}


/**
* @internal prvTgfCosEgressQoSdscpUpRemappingIpv4BuildPacket function
* @endinternal
*
* @brief   Build packet
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Packet description:
*       TGF_PACKET_PART_L2_E
*       TGF_PACKET_PART_VLAN_TAG_E
*       TGF_PACKET_PART_IPV4_E
*       TGF_PACKET_PART_PAYLOAD_E
*
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingIpv4BuildPacket
(
   GT_VOID
)
{
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;
    TGF_PACKET_VLAN_TAG_STC *vlanTagPartPtr;
    /* AUTODOC: get default IPv4 packet */
    rc = prvTgfPacketIpv4PacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_IPV4_E, &packetInfo, (GT_VOID *)&ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");

    ipv4PartPtr->totalLen = PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS;
    ipv4PartPtr->typeOfService = 0;
    /* AUTODOC: changed Ipv4 */
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_IPV4_E,0,ipv4PartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &packetInfo, (GT_VOID *)&vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
    vlanTagPartPtr->pri = 5;
    vlanTagPartPtr->vid = 5;
    rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,0,vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

    prvTgfIpv4PacketInfo.numOfParts = packetInfo.numOfParts;
    prvTgfIpv4PacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    prvTgfIpv4PacketInfo.partsArray =  packetInfo.partsArray;
}

/**
* @internal prvTgfCosEgressQoSdscpUpRemappingCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*
* @param[in] capturedPort             - port where output packet is captured.
* @param[in] testNumber               - test number
*                                       None
*/
void prvTgfCosEgressQoSdscpUpRemappingCheckCaptureEgressTrafficOnPort
(
    GT_U32 capturedPort,
    GT_U32 testNumber
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC expectedPacketInfo;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;
    TGF_VFD_INFO_STC                vfdArray[2];
    GT_U32                          numTriggers = 0;
    TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {0x8100, 4, 0, 5};
    TGF_PACKET_IPV4_STC *expIpv4PartPtr;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = capturedPort;
    if (testNumber == 2)
    {
        prvTgfPacketVlanTag1Part.pri = 6;
    }

    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);
    /* build expected packet */

    /* copy the expected packet from the ingress packet */
    expectedPacketInfo.numOfParts = prvTgfIpv4PacketInfo.numOfParts;
    expectedPacketInfo.totalLen = prvTgfIpv4PacketInfo.totalLen;
    /* allocate proper memory for the 'expected' parts*/
    expectedPacketInfo.partsArray =
        cpssOsMalloc(expectedPacketInfo.numOfParts * sizeof(TGF_PACKET_PART_STC));

    if(expectedPacketInfo.partsArray == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_PTR,
                                     "cpssOsMalloc: failed");
        return;
    }

    /* copy expected parts from the original sent parts */
    for(ii = 0 ; ii < expectedPacketInfo.numOfParts ; ii++)
    {
            expectedPacketInfo.partsArray[ii] = prvTgfIpv4PacketInfo.partsArray[ii];
            if (capturedPort == prvTgfPortsArray[1])
            {
                if (expectedPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
                {
                     expectedPacketInfo.partsArray[ii].partPtr = (GT_VOID*)&prvTgfPacketVlanTag1Part;
                }
            }
            if (capturedPort == prvTgfPortsArray[2])
            {
                if (expectedPacketInfo.partsArray[ii].type == TGF_PACKET_PART_IPV4_E)
                {
                    expIpv4PartPtr = expectedPacketInfo.partsArray[ii].partPtr;
                    if (testNumber == 1)
                    {
                        expIpv4PartPtr->typeOfService = 40; /* dscp =10 */
                    }
                    if (testNumber == 2)
                    {
                        expIpv4PartPtr->typeOfService = 244; /* dscp =61 */
                    }
                }
            }
    }


    /* Prepare VFD array entry for Identification field of IPv4 header */
    vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 23;
    vfdArray[0].cycleCount = sizeof(GT_U8);
    vfdArray[0].modeExtraInfo=0;
    vfdArray[0].incValue=1;
    vfdArray[0].patternPtr[0]=0;
    vfdArray[0].cycleCount=1;

    vfdArray[1].mode = TGF_VFD_MODE_DECREMENT_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 29;
    vfdArray[1].cycleCount = sizeof(GT_U8);
    vfdArray[1].modeExtraInfo=0;
    vfdArray[1].incValue=1;
    vfdArray[1].patternPtr[0]=0x66;
    vfdArray[1].cycleCount=1;


    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            &expectedPacketInfo,
            prvTgfBurstCount,/*numOfPackets*/
            2/*vfdNum*/,
            vfdArray /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            &numTriggers/*onFirstPacketNumTriggersBmpPtr*/);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* free the dynamic allocated memory */
    cpssOsFree(expectedPacketInfo.partsArray);
}


/**
* @internal prvTgfCosEgressQoSdscpUpRemappingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingTrafficGenerate
(
    IN GT_U32 testNumber
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;
    GT_U32          i = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* AUTODOC: send 1  IPv4  packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    for (i=1; i < 4; i++ )
    {
        if (!((testNumber == 2) && (i == 3)))
        {
            rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                            &prvTgfIpv4PacketInfo, prvTgfBurstCount, 0, NULL,
                            prvTgfDevNum,
                            prvTgfPortsArray[i],
                            TGF_CAPTURE_MODE_MIRRORING_E, 10);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                         prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
            /* check captured egress traffic on port */
            prvTgfCosEgressQoSdscpUpRemappingCheckCaptureEgressTrafficOnPort(prvTgfPortsArray[i],testNumber);
        }
    }

    return;
}

/**
* @internal prvTgfCosEgressQoSdscpUpRemappingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testNumber               - test number
*                                       None
*/
GT_VOID prvTgfCosEgressQoSdscpUpRemappingConfigurationRestore
(
    IN GT_U32 testNumber
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;
    GT_U8    up = 5; /* incoming up */
    GT_U32 mappingTableIndex = 1;
    GT_U8    cfiDeiBit = 0;
    GT_U32 tc = 3;
    CPSS_DP_LEVEL_ENT  dp = CPSS_DP_GREEN_E;


    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");
    /* restore QoS configuration */

    /* restore CPSS_QOS_PORT_TRUST_L2_E trust mode */
    rc = prvTgfCosPortQosTrustModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], modeGet);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                  "prvTgfCosPortQosTrustModeSet: %d, %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], modeGet);
    /* restore vlan tag1 to trust*/
    rc = prvTgfCosL2TrustModeVlanTagSelectSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE,vlanTagTypeGet);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
               "prvTgfCosL2TrustModeVlanTagSelectSet: %d, %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],CPSS_VLAN_TAG1_E);

      /* restore default eport trust mapping table select 1 */
     rc = prvTgfCosPortTrustQosMappingTableIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],GT_FALSE,mappingTableIndexGet);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortTrustQosMappingTableIndexSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

     /* restore mapped to up QoS profile in mapping table 1 */
     rc = prvTgfCosUpCfiDeiToProfileMapSet(mappingTableIndex,0, up, cfiDeiBit, profileIndex2UpGet);
     UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
               "prvTgfCosUpCfiDeiToProfileMapSet: %d, %d, %d, %d", prvTgfDevNum,up, cfiDeiBit,qosProfileIndex);

     /* restore QoS profile 1 entry */
    rc = prvTgfCosProfileEntrySet(qosProfileIndex, &profileEntryGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(
         GT_OK, rc, "prvTgfCosProfileEntrySet");

    /* restore QoS port attributes */
    rc = prvTgfCosPortQosConfigSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portQoSAttrCfgGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortQosConfigSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* egress QoS configuration */
    if (testNumber == 1)
    {
        for (i = 1; i < 3; i++)
        {
           /* restore egress QoS TC+DP enable state */
            rc = prvTgfChCosPortEgressQoStcDpMappingEnableSet(prvTgfDevNum, prvTgfPortsArray[i], egressQoStcDpEnable[i]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                      "prvTgfCosPortEgressQoSUpMappingEnableSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[i]);
        }
    }

    /* restore  egress QoS up enable state */
    rc = prvTgfCosPortEgressQoSUpMappingEnableSet(prvTgfDevNum, prvTgfPortsArray[1], egressQoSUpEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortEgressQoSUpMappingEnableSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[1]);

    /* restore egress QoS dscp enable state */
    rc = prvTgfCosPortEgressQoSDscpMappingEnableSet(prvTgfDevNum, prvTgfPortsArray[2], egressQoSDscpEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
              "prvTgfCosPortEgressQoSUpMappingEnableSet: %d, %d", prvTgfDevNum, prvTgfPortsArray[2]);

    for (i = 1; i < 3; i++)
    {
     /* restore egress eport  mapping table select */
     rc = prvTgfCosPortEgressQoSMappingTableIndexSet(prvTgfDevNum, prvTgfPortsArray[i],egressMappingTableIndexGet[i]);
     UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
               "prvTgfCosPortEgressQoSMappingTableIndexSet: %d, %d", prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    }
    if (testNumber == 1)
    {
    /* restore egress eport  mapping table select */
     rc = prvTgfCosEgressTcDp2UpExpDscpMappingEntrySet(prvTgfDevNum, 1, tc,dp,tcDp2Up,tcDp2Exp,tcDp2Dscp);
     UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    }

    if (testNumber == 2)
    {
    /* restore egress eport  up->up, dscp -> dscp mapping  */
         rc = prvTgfCosEgressUp2UpMappingEntrySet(prvTgfDevNum, 1, up, up2UpGet);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
         rc = prvTgfCosEgressDscp2DscpMappingEntrySet(prvTgfDevNum, 1, 0, dscp2DscpGet);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     *   Restore Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLAN entry 5 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);
}



