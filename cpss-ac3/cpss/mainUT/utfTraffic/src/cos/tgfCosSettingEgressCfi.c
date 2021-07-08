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
* @file tgfCosSettingEgressCfi.c
*
* @brief Egress CFI settings.
*
* @version   9
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
#include <cos/tgfCosSettingEgressCfi.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_5_CNS            5
#define PRV_TGF_VLANID_6_CNS            6

/* Traffic Classes used in test */
#define PRV_TGF_COS_TEST_TC0 3

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from */
#define PRV_TGF_EGR_PORT_IDX_CNS        3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* default value for ipv4 total length field */
#define PRV_TGF_IPV4_TOTAL_LENGTH_VALUE_CNS 0x54

/* pcl rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS        1

/***************** Tunnel config params  *******************************/

/* line index for the tunnel start entry in the router ARP / tunnel start table */
static GT_U32 prvTgfRouterArpTunnelStartLineIndex = 8;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfTunnelL2Part =
{
    {0x88, 0x99, 0x77, 0x66, 0x55, 0x88},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}     /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x01, 0x02, 0x03, 0x34, 0x02},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x25}     /* saMac */
};

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;
static GT_BOOL dpToCfiEnableGet;
static GT_U8   cfiDeiBitGet;
static GT_U16  ethertypeOrig[8];
static GT_U32  origPortTpidTag1Bmp;
static GT_U32  origTpidTag0BmpArr[8];
static GT_U32  origPortIngressTpidProfileTag0;
static PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT stateMode;
static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT vlanEgressPortTagState;
static TGF_PACKET_STC prvTgfIpv4PacketInfo;
static TGF_PACKET_STC prvTgfTunneledPacketInfo;
static TGF_PACKET_STC prvTgfEtherPacketInfo;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfCosSettingEgressTagCfiFieldBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*
* @param[in] testNumber               - number of test
* @param[in] vlanTagType              - vlan tag0 or vlan tag1
*                                       None
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldBridgeConfigSet
(
     GT_U32 testNumber,
     GT_U32 vlanTagType
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0, 0, 0, 0};
    GT_U32      tpidIndex = 0;
    GT_U32      i = 0;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* add tags */
    tagArray[0] = (GT_U8)GT_TRUE;
    tagArray[1] = (GT_U8)GT_TRUE;
    tagArray[2] = (GT_U8)GT_TRUE;
    tagArray[3] = (GT_U8)GT_TRUE;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* get ingress port ingress TPID profile for tag0 */
        rc = prvTgfBrgVlanPortIngressTpidProfileGet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                CPSS_VLAN_ETHERTYPE0_E,
                GT_TRUE,
                &origPortIngressTpidProfileTag0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileGet: %d", prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        
    }

    /* AUTODOC: create VLAN 5 with untagged\tagged ports [0,1] */
    if (testNumber == 5)
    {
        /* AUTODOC: get EtherType from all entries of TPID table */
        /* AUTODOC: set EtherType 0x8847 to all entries of TPID table */
        for (i = 0; i < 8; i++)
        {
            /* get EtherType in TPID table entry */
            rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,i,&ethertypeOrig[i]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                         prvTgfDevNum);
            /* AUTODOC: set ingress TPID entry 1 with EtherType 0x8847 */
            rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,i,TGF_ETHERTYPE_8847_MPLS_TAG_CNS);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                         prvTgfDevNum,TGF_ETHERTYPE_8847_MPLS_TAG_CNS);

        }
        /* AUTODOC: get all profiles for tag0 */
        /* AUTODOC: clear all profiles for tag0 */
        for (i = 0; i < 8; i++)
        {
            /* get ingress TPID profile bitmap for tag0 */
            rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_VLAN_ETHERTYPE0_E,
                    &origTpidTag0BmpArr[i]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d", prvTgfDevNum);

            /* clear ingress TPID tag0 profile bitmap */
            rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,i,CPSS_VLAN_ETHERTYPE0_E,0);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d, %x",
                                         prvTgfDevNum,CPSS_VLAN_ETHERTYPE0_E);
        }

        /* AUTODOC: set ingress TPID entry 1 with EtherType 0x9100 */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,TGF_ETHERTYPE_9100_TAG_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,TGF_ETHERTYPE_9100_TAG_CNS);

        /* AUTODOC: set bit 1 to TPID1 bmp for ingress port */
        rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_VLAN_ETHERTYPE0_E,
                    BIT_0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d", prvTgfDevNum);

        /* AUTODOC: Write VLAN Table Entry VID == 5, all ports tagged */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                               NULL, tagArray, 4);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    }
    else
    if (vlanTagType == 0)
    {
        /* AUTODOC: Write VLAN Table Entry VID == 5, all ports tagged */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_5_CNS, prvTgfPortsArray,
                                               NULL, tagArray, 4);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    }
    else
    if (vlanTagType == 1)
    {
        /* AUTODOC: Write VLAN Table Entry VID == 5, all ports TAG1 tagged */
        rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
            PRV_TGF_VLANID_5_CNS, PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");
        /* AUTODOC: get EtherType from all entries of TPID table */
        /* AUTODOC: set EtherType 0x8847 to all entries of TPID table */
        for (i = 0; i < 8; i++)
        {
            /* get EtherType in TPID table entry */
            rc = prvTgfBrgVlanTpidEntryGet(CPSS_DIRECTION_INGRESS_E,i,&ethertypeOrig[i]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntryGet: %d",
                                         prvTgfDevNum);
            /* AUTODOC: set ingress TPID entry 1 with EtherType 0x8847 */
            rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,i,TGF_ETHERTYPE_8847_MPLS_TAG_CNS);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                         prvTgfDevNum,TGF_ETHERTYPE_8847_MPLS_TAG_CNS);

        }
        /* AUTODOC: get all profiles for tag0 */
        /* AUTODOC: clear all profiles for tag0 */
        for (i = 0; i < 8; i++)
        {
            /* get ingress TPID profile bitmap for tag0 */
            rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_VLAN_ETHERTYPE0_E,
                    &origTpidTag0BmpArr[i]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d", prvTgfDevNum);

            /* clear ingress TPID tag0 profile bitmap */
            rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,i,CPSS_VLAN_ETHERTYPE0_E,0);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d, %x",
                                         prvTgfDevNum,CPSS_VLAN_ETHERTYPE0_E);
        }

        /* AUTODOC: set ingress TPID entry 1 with EtherType 0x8100 */
        rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,tpidIndex,TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                     prvTgfDevNum,TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

        rc = prvTgfBrgVlanPortIngressTpidGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_VLAN_ETHERTYPE1_E,
                    &origPortTpidTag1Bmp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidGet: %d", prvTgfDevNum);

        /* AUTODOC: set bit 1 to TPID1 bmp for ingress port */
        rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    CPSS_VLAN_ETHERTYPE1_E,
                    BIT_0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d", prvTgfDevNum);

        /* save VLAN egress port tag state mode  for egress ePort 54*/
        rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                    &stateMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanEgressPortTagStateModeGet: %d", prvTgfDevNum);

        /* AUTODOC: set EPORT tag state mode for egress ePort 54 */
        rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanEgressPortTagStateModeSet: %d", prvTgfDevNum);


        /* save VLAN egress ePort 54 tag state */
        rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                &vlanEgressPortTagState);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanEgressPortTagStateGet: %d", prvTgfDevNum);

        /* AUTODOC: set TAG1_CMD port tag state for egress ePort 54 */
        rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanEgressPortTagStateSet: %d", prvTgfDevNum);

    }
    if (testNumber == 3)
    {
        /* set default vlan entry for vlan ID 6 */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_6_CNS, prvTgfPortsArray + 2,
                                               NULL, NULL, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

        /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
        rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_5_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);
    }
}




/**
* @internal prvTgfCosSettingEgressTagCfiFieldDpToCfiMappingSet function
* @endinternal
*
* @brief   Set DP to CFI mapping configuration
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldDpToCfiMappingSet
(
     GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: DP to CFI CONFIGURATION: */
    /* Get DP to CFI mapping enable status */
    rc = prvTgfCosPortDpToCfiDeiMapEnableGet(prvTgfDevNum, prvTgfPortsArray[3], &dpToCfiEnableGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfDevNum, prvTgfPortsArray[3]);


     /* AUTODOC: enable DP to CFI mapping: */
    PRV_UTF_LOG0_MAC("======= enable DP to CFI mapping =======\n");
    rc = prvTgfCosPortDpToCfiDeiMapEnableSet(prvTgfDevNum, prvTgfPortsArray[3], GT_TRUE);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfDevNum, prvTgfPortsArray[3]);

    /* Get dp to CFI mapping */
    rc = prvTgfCosDpToCfiDeiMapGet(prvTgfDevNum, CPSS_DP_GREEN_E, &cfiDeiBitGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);

    /* AUTODOC: set DP to CFI mapping: */
    PRV_UTF_LOG0_MAC("======= set DP to CFI mapping =======\n");
    rc = prvTgfCosDpToCfiDeiMapSet(CPSS_DP_GREEN_E,1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
}




/**
* @internal prvTgfCosSettingEgressTagCfiFieldBuildPacket function
* @endinternal
*
* @brief   Build packet
*
* @param[in] testNumber               - number of test
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
GT_VOID prvTgfCosSettingEgressTagCfiFieldBuildPacket
(
   GT_U32 testNumber
)
{
    TGF_PACKET_STC          packetInfo;
    GT_STATUS               rc = GT_OK;
    TGF_PACKET_IPV4_STC     *ipv4PartPtr;
    TGF_PACKET_VLAN_TAG_STC *vlanTagPartPtr;
    GT_U32 ii;

    if (testNumber == 5)
    {
        /* set number vlan tags in passenger */
        rc = prvTgfPacketNumberOfVlanTagsSet(GT_TRUE, 2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

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
    }
    else
    if (testNumber > 2)
    {
        /* set number vlan tags in passenger */
        rc = prvTgfPacketNumberOfVlanTagsSet(GT_TRUE, 2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

        /* set number vlan tags in packet */
        rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE, 2);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

        /* AUTODOC: get default Ethernet packet */
        rc = prvTgfPacketEthernetPacketDefaultPacketGet(&packetInfo.numOfParts, &packetInfo.partsArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketDefaultPacketGet");

        /* remove ethertype part */
        ii = 0;
        while (packetInfo.partsArray[ii].type != TGF_PACKET_PART_ETHERTYPE_E)
        {
            ii++;
            if (packetInfo.partsArray[ii].type == TGF_PACKET_PART_PAYLOAD_E)
                break;
        }
        if (packetInfo.partsArray[ii].type == TGF_PACKET_PART_ETHERTYPE_E)
            packetInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;

    }
    else
    {
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
    }
    rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &packetInfo, (GT_VOID *)&vlanTagPartPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
    vlanTagPartPtr->vid = 5;
    switch (testNumber)
    {
        case 1:
            vlanTagPartPtr->cfi = 1;
            break;
        case 5:
            vlanTagPartPtr->etherType = TGF_ETHERTYPE_9100_TAG_CNS;
            vlanTagPartPtr->cfi = 1;
            break;
        default:
            vlanTagPartPtr->cfi = 0;
            break;
    }
    vlanTagPartPtr->pri = 5;
    if ((testNumber < 3) ||
        (testNumber == 5))
    {

        rc = prvTgfPacketIpv4PacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,0,vlanTagPartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");
        prvTgfIpv4PacketInfo.numOfParts = packetInfo.numOfParts;
        prvTgfIpv4PacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
        prvTgfIpv4PacketInfo.partsArray =  packetInfo.partsArray;
    }
    else
    {
        rc = prvTgfPacketEthernetPacketHeaderDataSet(TGF_PACKET_PART_VLAN_TAG_E,0,vlanTagPartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpv4PacketHeaderDataSet");

        prvTgfEtherPacketInfo.numOfParts = packetInfo.numOfParts;
        prvTgfEtherPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
        prvTgfEtherPacketInfo.partsArray =  packetInfo.partsArray;
    }
}

/**
* @internal prvTgfCosSettingEgressTagCfiFieldCheckCaptureEgressTrafficOnPort function
* @endinternal
*
* @brief   check captured egress on specific port of the test.
*
* @param[in] capturedPort             - port where output packet is captured.
* @param[in] testNumber               - test number
* @param[in] cfiValue                 - CFI bit
*                                       None
*/
void prvTgfCosSettingEgressTagCfiFieldCheckCaptureEgressTrafficOnPort
(
    GT_U32 capturedPort,
    GT_U32 testNumber,
    GT_U32 cfiValue
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_PACKET_STC expectedPacketInfo;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/
    GT_U32  ii;
    TGF_VFD_INFO_STC                vfdArray[2];
    GT_U32                          numTriggers = 0;
    TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {0x8100, 5, 0, 5};
    TGF_PACKET_VLAN_TAG_STC prvTgfTunnelVlanTagPart = {0x8100,5,1,6};
    GT_U32  byteNumMaskList[] = { 22, 23, 28, 29 }; /* list of bytes for which
                                                       the comparison is prohibited */
    GT_U32  byteNumMaskList1[] = { 18, 19, 24, 25 }; /* list of bytes for which
                                                       the comparison is prohibited */
    /* packet's IPv4 over IPv4 */
    TGF_PACKET_IPV4_STC prvTgfPacketIpv4TunnelPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x6C,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x28,               /* timeToLive */
    0x2F,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
   {10, 10, 10, 10},   /* srcAddr */
   { 4,  4,  4,  4}   /* dstAddr */
};


    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = capturedPort;
    prvTgfPacketVlanTag1Part.cfi = (TGF_CFI)cfiValue;
    PRV_UTF_LOG1_MAC("check Port [%d] capturing:\n", portInterface.devPort.portNum);
    /* build expected packet */
    if ((testNumber >=3 ) &&
        (testNumber != 5))
    {
        /* expected packet */
        /* set number vlan tags in tunnel */
        rc = prvTgfPacketNumberOfVlanTagsSet(GT_FALSE, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketNumberOfVlanTagsSet");

        prvTgfPacketIsGreSet(GT_TRUE);
        /* AUTODOC: get default Eth over IPv4 GRE packet */
        rc = prvTgfPacketEthOverIpv4PacketDefaultPacketGet(&prvTgfTunneledPacketInfo.numOfParts, &prvTgfTunneledPacketInfo.partsArray);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthOverIpv4PacketDefaultPacketGet");

        prvTgfTunneledPacketInfo.totalLen = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    }

    if ((testNumber == 2) || (testNumber == 3))
    {
        prvTgfPacketVlanTag1Part.cfi = 1;
    }

    if ((testNumber < 3) ||
        (testNumber == 5))
    {
        /* copy the expected packet from the ingress packet */
        expectedPacketInfo.numOfParts = prvTgfIpv4PacketInfo.numOfParts;
        expectedPacketInfo.totalLen = prvTgfIpv4PacketInfo.totalLen;
    }
    else
    {
        /* copy the expected packet from the ingress packet */
        expectedPacketInfo.numOfParts = prvTgfTunneledPacketInfo.numOfParts;
        expectedPacketInfo.totalLen = prvTgfTunneledPacketInfo.totalLen;
    }
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
        if (testNumber < 3)
        {
            expectedPacketInfo.partsArray[ii] = prvTgfIpv4PacketInfo.partsArray[ii];
            if (expectedPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E)
            {
                 expectedPacketInfo.partsArray[ii].partPtr = (GT_VOID*)&prvTgfPacketVlanTag1Part;
            }
        }
        else
        if (testNumber == 5)
        {
            expectedPacketInfo.partsArray[ii] = prvTgfIpv4PacketInfo.partsArray[ii];
            if ((expectedPacketInfo.partsArray[ii].type == TGF_PACKET_PART_VLAN_TAG_E) &&
                ((ii == 0) ||
                 (expectedPacketInfo.partsArray[ii-1].type != TGF_PACKET_PART_VLAN_TAG_E)))
            {
                expectedPacketInfo.partsArray[ii].partPtr = (GT_VOID*)&prvTgfPacketVlanTag1Part;
            }
        }
        else
        {
            expectedPacketInfo.partsArray[ii] = prvTgfTunneledPacketInfo.partsArray[ii];
            if (ii == 0)
            {
                /* change tunnel L2 part in expected packet */
                expectedPacketInfo.partsArray[ii].partPtr = (void*)&prvTgfTunnelL2Part;
            }
            if (ii == 1)
            {
                /* change tunnel vlan tag part in expected packet */
                if (testNumber == 3)
                {
                    expectedPacketInfo.partsArray[ii].partPtr = (GT_VOID*)&prvTgfTunnelVlanTagPart;
                }
                else
                {
                    expectedPacketInfo.partsArray[ii].type = TGF_PACKET_PART_SKIP_E;
                }
            }
            if (ii == 6)
            {
                expectedPacketInfo.partsArray[ii].partPtr = (GT_VOID*)&prvTgfPacketIpv4TunnelPart;

            }
            if (ii == 9)
            {
                expectedPacketInfo.partsArray[ii].partPtr = prvTgfEtherPacketInfo.partsArray[0].partPtr;
            }
            if (ii == 10)
            {
                prvTgfPacketVlanTag1Part.cfi = 0;
                expectedPacketInfo.partsArray[ii].partPtr = (GT_VOID*)&prvTgfPacketVlanTag1Part;
            }
            if (ii == 11)
            {
                 expectedPacketInfo.partsArray[ii].partPtr = prvTgfEtherPacketInfo.partsArray[2].partPtr;
            }
        }
    }

    switch (testNumber)
    {
    case 3:
        /* print and check captured packets */
        rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                &portInterface,
                &expectedPacketInfo,
                prvTgfBurstCount,/*numOfPackets*/
                0/*vfdNum*/,
                NULL /*vfdArray*/,
                byteNumMaskList, /* skip check of bytes in these positions */
                sizeof(byteNumMaskList)/sizeof(byteNumMaskList[0]), /* length of skip list */
                &actualCapturedNumOfPackets,
                NULL/*onFirstPacketNumTriggersBmpPtr*/);
        break;
    case 4:
        /* print and check captured packets */
        rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                &portInterface,
                &expectedPacketInfo,
                prvTgfBurstCount,/*numOfPackets*/
                0/*vfdNum*/,
                NULL /*vfdArray*/,
                byteNumMaskList1, /* skip check of bytes in these positions */
                sizeof(byteNumMaskList1)/sizeof(byteNumMaskList1[0]), /* length of skip list */
                &actualCapturedNumOfPackets,
                NULL/*onFirstPacketNumTriggersBmpPtr*/);
        break;
    default:
        /* print captured packets and check TriggerCounters */
        rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                &portInterface,
                &expectedPacketInfo,
                prvTgfBurstCount,/*numOfPackets*/
                0/*vfdNum*/,
                vfdArray /*vfdArray*/,
                NULL, /* bytesNum's skip list */
                0,    /* length of skip list */
                &actualCapturedNumOfPackets,
                &numTriggers/*onFirstPacketNumTriggersBmpPtr*/);
        break;
    }


    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

    /* free the dynamic allocated memory */
    cpssOsFree(expectedPacketInfo.partsArray);
}


/**
* @internal prvTgfCosSettingEgressTagCfiFieldTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:34:02,
*         macSa = 00:00:00:00:00:01,
* @param[in] testNumber               - test number
* @param[in] vlanTagType              - vlan tag type
*                                       None
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldTrafficGenerate
(
    IN GT_U32 testNumber,
    IN GT_U32 vlanTagType
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          portsCount  = PRV_TGF_PORT_COUNT_CNS;
    GT_U32          portIter    = 0;
    GT_U32          cfi_value;
    TGF_PACKET_VLAN_TAG_STC *vlanTagPartPtr;
    TGF_PACKET_STC          *packetInfoPtr = &prvTgfIpv4PacketInfo;
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
    switch (testNumber)
    {
    case 1:
    case 5:
        cfi_value = 1;
        packetInfoPtr = &prvTgfIpv4PacketInfo;
        /* -------------------------------------------------------------------------
         * 2. Generating Traffic
         */

        /* AUTODOC: send 1  IPv4  packets from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
        /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
        rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                        packetInfoPtr, prvTgfBurstCount, 0, NULL,
                        prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                        TGF_CAPTURE_MODE_MIRRORING_E, 10);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* check captured egress traffic on port */
        prvTgfCosSettingEgressTagCfiFieldCheckCaptureEgressTrafficOnPort(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],testNumber,cfi_value);
        break;
    default:
        break;
    }

    cfi_value = 0;
    if ((vlanTagType == 0) && ((testNumber < 3) || (testNumber == 5)))
    {
        rc =  prvTgfBrgGenCfiRelayEnableSet(prvTgfDevNum,GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenCfiRelayEnableSet: %d",
                                     prvTgfDevNum);
    }

    if (vlanTagType == 1)
    {
        /* change cfi bit in incoming packet to 0*/
        rc = prvTgfPacketHeaderPartGet(TGF_PACKET_PART_VLAN_TAG_E, &prvTgfIpv4PacketInfo, (GT_VOID *)&vlanTagPartPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketHeaderPartGet");
        vlanTagPartPtr->cfi = 0;
    }
    if ((testNumber > 2) &&
        (testNumber != 5))
    {
        packetInfoPtr = &prvTgfEtherPacketInfo;
    }
    /* -------------------------------------------------------------------------
     * 3. Generating Traffic
     */

    /* AUTODOC: send 1  IPv4  packets from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=10.10.10.10, dstIP=4.4.4.4 */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                    packetInfoPtr, prvTgfBurstCount, 0, NULL,
                    prvTgfDevNum,
                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                    TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* check captured egress traffic on port */
    prvTgfCosSettingEgressTagCfiFieldCheckCaptureEgressTrafficOnPort(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],testNumber,cfi_value);


    return;
}


/**
* @internal prvTgfCosSettingEgressTagCfiFieldPclConfigSet function
* @endinternal
*
* @brief   Set PCL configuration
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldPclConfigSet
(
     GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, LOOKUP_0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /******* set PCL rule - FORWARD packet with MAC DA = ... 34 02 *******/
    /* clear mask */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* clear pattern */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* clear action */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* setup action */
    action.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    action.redirect.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;

    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType =
                            PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;
    action.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr =
                                            prvTgfRouterArpTunnelStartLineIndex;
    action.bypassBridge = GT_TRUE;/* (sip5) must be set explicitly */

    /* setup pattern */
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(pattern.ruleStdNotIp.macDa.arEther));

    /* setup mask */
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleStdNotIp.macDa.arEther));

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP, cmd=FORWARD */
    /* AUTODOC:   redirectCmd=REDIRECT_CMD_OUT_IF, port=2  */
    /* AUTODOC:   outlifType=TUNNEL, tunnelType=ETHERNET, tsPtr=8 */
    /* AUTODOC:   pattern DA=00:00:00:00:34:02 */
    rc = prvTgfPclRuleSet(ruleFormat, PRV_TGF_PCL_RULE_IDX_CNS,
                          &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 prvTgfDevNum, ruleFormat);

}

/**
* @internal prvTgfCosSettingEgressTagCfiFieldTunnelConfigSet function
* @endinternal
*
* @brief   Build packet
*
* @param[in] testNumber               - number of test
* @param[in] tunneVlanTagState        - tunnel vlan tag state
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
GT_VOID prvTgfCosSettingEgressTagCfiFieldTunnelConfigSet
(
    GT_U32 testNumber,
    GT_BOOL tunneVlanTagState

)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    /* clear tunnelEntry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;

    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable    = tunneVlanTagState;
    tunnelEntry.ipv4Cfg.vlanId       = PRV_TGF_VLANID_6_CNS;
    tunnelEntry.ipv4Cfg.up           = 5;
    tunnelEntry.ipv4Cfg.upMarkMode   = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.upMarkMode   = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscpMarkMode = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.ttl          = 40;
    tunnelEntry.ipv4Cfg.ethType      = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelEntry.ipv4Cfg.greProtocolForEthernet = 0x6558;

    cpssOsMemSet((GT_VOID*) &tunnelEntry.ipv4Cfg.srcIp.arIP, 0xA,
                 sizeof(tunnelEntry.ipv4Cfg.srcIp.arIP));
    cpssOsMemSet((GT_VOID*) &tunnelEntry.ipv4Cfg.destIp.arIP, 4,
                 sizeof(tunnelEntry.ipv4Cfg.destIp.arIP));

    /* tunnel next hop MAC DA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther,
                 &prvTgfTunnelL2Part.daMac, sizeof(prvTgfTunnelL2Part.daMac));

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=X_OVER_GRE_IPV4 */
    /* AUTODOC:   vlanId=5, ttl=40, ethType=TUNNEL_GRE0_ETHER_TYPE */
    /* AUTODOC:   upMarkMode=dscpMarkMode=MARK_FROM_ENTRY */
    /* AUTODOC:   DA=88:99:77:66:55:88, srcIp=10.10.10.10, dstIp=4.4.4.4 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex,
                                   tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* clear tunnelEntry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex,
                                   &tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    if (testNumber == 3)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_VLANID_6_CNS, tunnelEntry.ipv4Cfg.vlanId,
                         "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);
    }

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelL2Part.daMac,
                      sizeof(prvTgfPacketL2Part.daMac)) == 0 ?  GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                            tunnelEntry.ipv4Cfg.macDa.arEther[5]);

}


/**
* @internal prvTgfCosSettingEgressTagCfiFieldConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*
* @param[in] testNumber               - test number
* @param[in] vlanTagType              - vlan tag type
*                                       None
*/
GT_VOID prvTgfCosSettingEgressTagCfiFieldConfigurationRestore
(
    IN GT_U32 testNumber,
    IN GT_U32 vlanTagType
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    if (vlanTagType == 0)
    {
        /* restore CFI bit */
        rc =  prvTgfBrgGenCfiRelayEnableSet(prvTgfDevNum,GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenCfiRelayEnableSet: %d",
                                     prvTgfDevNum);
    }
    if (testNumber == 5)
    {
        for (i = 0; i < 8; i++)
        {
            /* AUTODOC: restore ingress TPID entry  with original EtherType  */
            rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,i,ethertypeOrig[i]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                         prvTgfDevNum,ethertypeOrig[i]);
            /* AUTODOC: restore ingress TPID tag0 profile bitmap */
            rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,i,CPSS_VLAN_ETHERTYPE0_E,origTpidTag0BmpArr[i]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d, %x",
                                         prvTgfDevNum,CPSS_VLAN_ETHERTYPE0_E);
        }
    }
    else
     if (vlanTagType == 1)
     {
         for (i = 0; i < 8; i++)
         {
             /* AUTODOC: restore ingress TPID entry  with original EtherType  */
             rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E,i,ethertypeOrig[i]);
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTpidEntrySet: %d, %x",
                                          prvTgfDevNum,ethertypeOrig[i]);
             /* AUTODOC: restore ingress TPID tag0 profile bitmap */
             rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,i,CPSS_VLAN_ETHERTYPE0_E,origTpidTag0BmpArr[i]);
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressTpidProfileSet: %d, %x",
                                          prvTgfDevNum,CPSS_VLAN_ETHERTYPE0_E);
         }
         /* AUTODOC: restore tag1 TPID bmp for ingress port */
         rc = prvTgfBrgVlanPortIngressTpidSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                     CPSS_VLAN_ETHERTYPE1_E,
                     origPortTpidTag1Bmp);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidSet: %d", prvTgfDevNum);
         /* AUTODOC: restore EPORT tag state mode for egress ePort 54 */
         rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                     prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                     stateMode);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanEgressPortTagStateModeSet: %d", prvTgfDevNum);
         /* AUTODOC: restore TAG1_CMD port tag state for egress ePort 54 */
         rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                                 prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                 vlanEgressPortTagState);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanEgressPortTagStateSet: %d", prvTgfDevNum);


     }
    if ((testNumber == 2) || (testNumber == 3))
    {
        /* restore dp to CFI mapping status */
        rc = prvTgfCosPortDpToCfiDeiMapEnableSet(prvTgfDevNum, prvTgfPortsArray[3], dpToCfiEnableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfDevNum, prvTgfPortsArray[3]);
        /* restore dp to CFI mapping  */
        rc = prvTgfCosDpToCfiDeiMapSet(CPSS_DP_GREEN_E,cfiDeiBitGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    }
    prvTgfPacketRestoreDefaultParameters();

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
    if ((testNumber > 2) &&
        (testNumber != 5))
    {
        /* AUTODOC: invalidate VLAN entry 6 */
        rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);
        /* AUTODOC: disable ingress policy on port 0 */
        rc = prvTgfPclPortIngressPolicyEnable(
                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfPclPortIngressPolicyEnable: %d", prvTgfDevNum);

        /* AUTODOC: invalidate PCL rule 1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                         PRV_TGF_PCL_RULE_IDX_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfPclRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* set ingress port ingress TPID profile for tag0 */
        rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                CPSS_VLAN_ETHERTYPE0_E,
                GT_TRUE,
                origPortIngressTpidProfileTag0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngressTpidProfileSet: %d", prvTgfDevNum,prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        
    }
}



